#include "BoardConverter.h"



/* ---------------------------------------------------------------------------
 *  Compact 5-bit encoding for a 3-square Tic-Tac-Toe row
 *
 *  The board row is stored in a single uint8_t produced by rowToFiveBit() and
 *  read back by fiveBitsToRow().  This gives us a loss-free representation of
 *  every possible pattern (3^3 = 27) while using only 5 of the 8 bits.
 *
 *  +-----------------------------------------------+
 *  | Bit-layout (LSB first)                        |
 *  +----+------------------------------------------+
 *  | 0-2| OCCUPANCY MASK - one bit per square      |
 *  |    |    0 -> square is empty                  |
 *  |    |    1 -> square is filled (X or O)        |
 *  | 3-4| META  - meaning depends on occupancy     |
 *  +----+------------------------------------------+
 *
 *  A. Generic rows (1-2 occupied squares)
 *     > The META bits are consumed from left to right, one per *occupied*
 *       square.  0 = that square is 'O', 1 = that square is 'X'.
 *     > Because there are only two META bits, this path is used only when
 *       <= 2 squares are occupied.
 *
 *  B. Dense rows (all 3 squares occupied)
 *     > The generic layout cannot express three X/O flags, so a special
 *       encoding is used instead:
 *
 *         OCCUPANCY = 111  -> assume the row is *all X*.
 *                          META gives the index (0-2) of a *single O*,
 *                          or the value 3 to mean “really all X”.
 *
 *         OCCUPANCY = 000  -> assume the row is *all O*.
 *                          META gives the index (0-2) of a *single X*,
 *                          or the value 3 to mean “really all O”.
 *
 *  C. Empty row
 *     PATTERN_EMPTY (all five bits zero) is reserved for the case in which
 *     no squares are occupied.
 *
 *  Constants
 *  ----------
 *      META_START     = (1u << 3)          // first META bit (bit 3)
 *      PATTERN_EMPTY  = 0b10001            // completely empty row
 *
 *  Guarantees
 *  ----------
 *      > The top three bits of the returned uint8_t are always zero.
 *      > Every legal Tic-Tac-Toe row maps to a unique 5-bit pattern and back.
 *
 *  rowToFiveBit(Square row[3])
 *      Encodes the row into the 5-bit pattern described above.
 *
 *  fiveBitsToRow(uint8_t bits, Square row[3])
 *      Decodes the 5-bit pattern back into an array of Squares.
 * ------------------------------------------------------------------------- */


uint8_t rowToFiveBit(const Square(&row)[3]) {
	using namespace detail;

	int squaresSelectedCount = 0;
	uint8_t bitPattern = 0;

	uint8_t xCount = 0;
	int8_t xLast = 3;

	uint8_t oCount = 0;
	int8_t oLast = 3;

	for (uint8_t i = 0; i < 3; i++) {
		if (row[i] == Square::X) bitPattern |= META_START << squaresSelectedCount;
		bool isSet = (row[i] != Square::none);
		squaresSelectedCount += isSet;
		bitPattern |= isSet << i;
		if (row[i] == Square::X) {
			xCount++;
			xLast = i;
		}
		if (row[i] == Square::O) {
			oCount++;
			oLast = i;
		}
	}
	if(squaresSelectedCount == 0) return PATTERN_EMPTY; //No square selected
	if (squaresSelectedCount == 3) {
		bitPattern = 0b00'000 | xLast << 3;
		if(oCount <= 1) bitPattern = 0b00'111 | oLast << 3;
	}
	return bitPattern & 0b11'111;
}

void fiveBitsToRow(uint8_t bits, Square(&row)[3]) {
	using namespace detail;

	if (bits == PATTERN_EMPTY) {
		row[0] = row[1] = row[2] = Square::none;
		return;
	}
	if ((bits & 0b111) == 0b111) {
		row[0] = row[1] = row[2] = Square::X;
		int num = (bits >> 3) & 0b11;
		if (num < 3) row[num] = Square::O;
		return;
	}
	if ((bits & 0b111) == 0b000) {
		row[0] = row[1] = row[2] = Square::O;
		int num = (bits >> 3) & 0b11;
		if (num < 3) row[num] = Square::X;
		return;
	}

	// Generic case
	int metaIdx = 0;

	for (int i = 0; i < 3; ++i) {
		bool occupied = bits & (1 << i);
		if (!occupied) {
			row[i] = Square::none;
			continue;
		} else {
			row[i] = Square::O;
			if(bits & (META_START << metaIdx)) row[i] = Square::X;
			metaIdx++;
		}
	}
}


/* -----------------------------------------------------------------------------
 *  Packing a stream of boards into a compact byte buffer (15 bits per board)
 *
 *  Overview
 *  --------
 *  Each Tic-Tac-Toe board row is encoded by rowToFiveBit() into 5 bits.
 *  A Board has 3 rows, so one Board = 15 bits. The functions below pack and
 *  unpack an arbitrary sequence of boards as a dense bitstream with no padding.
 *
 *  Layout & Ordering
 *  -----------------
 *    -> Rows are written row-major within a board: r=0,1,2.
 *    -> Boards are concatenated in the order provided by the BoardStream.
 *    -> Bits are packed LSB-first within each byte (bit 0 is the least-significant
 *      bit of data[0]). There is no alignment or padding between rows/boards.
 *
 *  Byte size
 *  ---------
 *    boardsToMemoryBlock:
 *      totalBits  = boards.size() * 15
 *      totalBytes = ceil(totalBits / 8) = (totalBits + 7) >> 3
 *      The returned ByteVector is zero-initialized so we can OR-in partial bytes.
 *
 *  Writing (boardsToMemoryBlock)
 *  -----------------------------
 *    For each 5-bit row code:
 *      -> bitPos   : running bit offset in the output stream.
 *      -> byteIdx  = bitPos >> 3            (current byte)
 *      -> bitIdx   = bitPos & 7             (bit offset inside that byte)
 *      -> scratch  = uint16_t(rowBits) << bitIdx
 *        - Write low 8 bits into data[byteIdx].
 *        - If the 5-bit field crosses the byte boundary (bitIdx > 3),
 *          also write the overflow (scratch >> 8) into data[byteIdx + 1].
 *      -> bitPos  += 5.
 *
 *  Reading (memoryBlockToBoards)
 *  -----------------------------
 *    For each row to recover:
 *      -> byteIdx  = bitPos >> 3
 *      -> bitIdx   = bitPos & 7
 *      -> Assemble a 16-bit window: scratch = data[byteIdx] |
 *        (byteIdx + 1 < byteCount ? uint16_t(data[byteIdx+1]) << 8 : 0)
 *      -> rowBits  = (scratch >> bitIdx) & 0b1'1111  // take 5 bits
 *      -> Decode via fiveBitsToRow(rowBits, board.squares[r]).
 *      -> bitPos  += 5.
 *    Boards are pushed to the result in order.
 *
 *  Endianness
 *  ----------
 *    The on-wire format is defined in terms of byte arrays and bit positions.
 *    It does not depend on CPU endianness.
 *
 *  Preconditions / Notes
 *  ---------------------
 *    -> memoryBlockToBoards assumes (boardCount * 15) bits are present in [data,
 *      byteCount). If the final row would spill past byteCount, decoding will
 *      lose high bits. Ensure the buffer length was computed as above.
 *    -> Unused high bits of the last byte (if totalBits % 8 != 0) are zero in
 *      boardsToMemoryBlock’s output.
 *
 *  Complexity
 *  ----------
 *    O(N) time where N = number of boards; contiguous O(totalBytes) storage.
 * --------------------------------------------------------------------------- */

ByteVector boardsToMemoryBlock(const BoardStream& boards) {
	const size_t totalBits = boards.size() * 15;
	const size_t totalBytes = (totalBits + 7) >> 3;   // ceil(bits/8)

	ByteVector data(totalBytes, 0);
	size_t bitPos = 0;

	for (const Board& b: boards)
	{
		for (int r = 0; r < 3; ++r)
		{
			const std::uint8_t rowBits = rowToFiveBit(b.squares[r]);

			const size_t byteIdx = bitPos >> 3;
			const size_t bitIdx = bitPos & 7;

			std::uint16_t scratch = static_cast<std::uint16_t>(rowBits) << bitIdx;

			data[byteIdx] |= scratch & 0xFF;
			if (bitIdx > 3) data[byteIdx + 1] |= scratch >> 8;
			bitPos += 5;
		}
	}
	return data;
}

BoardStream memoryBlockToBoards(const std::uint8_t* data, size_t byteCount, size_t boardCount) {
	BoardStream boards;
	boards.reserve(boardCount);

	size_t bitPos = 0;
	for (size_t b = 0; b < boardCount; ++b)
	{
		Board board{};
		for (int r = 0; r < 3; ++r)
		{
			const size_t byteIdx = bitPos >> 3;
			const size_t bitIdx = bitPos & 0b111;

			std::uint16_t scratch = data[byteIdx];
			if (byteIdx + 1 < byteCount)
				scratch |= static_cast<std::uint16_t>(data[byteIdx + 1]) << 8;

			const std::uint8_t rowBits = (scratch >> bitIdx) & 0b11111;
			fiveBitsToRow(rowBits, board.squares[r]);

			bitPos += 5;
		}
		boards.push_back(board);
	}
	return boards;
}
