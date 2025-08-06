#include "HuffmanTree.h"

uint16_t HuffmanTree::getBoardAtPos(const uint8_t* data, size_t bytes, size_t bitPos) {
    size_t byteIdx = bitPos >> 3;
    size_t bitIdx = bitPos & 0b111;

    uint32_t scratch = data[byteIdx]
        | (static_cast<uint32_t>(data[byteIdx + 1]) << 8)
        | (byteIdx + 2 < bytes ? (static_cast<uint32_t>(data[byteIdx + 2]) << 16) : 0);

    return (scratch >> bitIdx) & 0b11111'11111'11111;
}

void HuffmanTree::writeBoardAtPos(ByteVector& data, size_t bitPos, uint16_t board, size_t size) {
    size_t byte = bitPos >> 3;
    size_t byteTop = (bitPos + size - 1) >> 3;
    if (data.size() < byteTop + 1) {
        data.resize(byteTop + 1, 0);
    }
    uint32_t scratch = (uint32_t(board) << (bitPos & 0b111));
    data[byte] |= scratch & 0b1111'1111;
    data[byte + 1] = (scratch >> 8) & 0b1111'1111;
    if (byteTop == byte + 2) data[byteTop] = (scratch >> 16) & 0b1111'1111;
}


void HuffmanTree::DFSC(std::shared_ptr<Node> node, ByteVector& data, size_t& bitPos) {
    if (node->childOne == nullptr) {
        writeBoardAtPos(data, bitPos, (node->value << 1) + 1, 16);
        bitPos += 16;
        return;
    }
    bitPos++;
    DFSC(node->childOne, data, bitPos);
    DFSC(node->childTwo, data, bitPos);
}

bool HuffmanTree::DFSE(const std::shared_ptr<Node> node, const std::shared_ptr<Node> otherNode) const {
    if (node->childOne == nullptr) {
        return node->value == otherNode->value;
    }
    if (otherNode->childOne == nullptr) return false;
    if (otherNode->childTwo == nullptr) return false;
    return DFSE(node->childOne, otherNode->childOne) && DFSE(node->childTwo, otherNode->childTwo);
}

HuffmanTree::HuffmanTree(const ByteVector& raw) {
    // 1. Build frequency map
    std::unordered_map<uint16_t, size_t> freq;
    const size_t totalBits = raw.size() * 8;
    for (size_t bitPos = 0; bitPos + 15 <= totalBits; bitPos += 15) {
        ++freq[getBoardAtPos(raw.data(), raw.size(), bitPos)];
    }

    // 2. Build min-heap of nodes
    struct PQEntry {
        size_t frequency = 0;
        std::shared_ptr<Node> node;
        bool operator>(const PQEntry& other) const { return frequency > other.frequency; }
    };
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<>> pq;

    for (auto keyValuePair : freq) {
        auto node = std::make_shared<Node>();
        node->value = keyValuePair.first;
        this->leafNodes[node->value] = node;
        PQEntry pqe;
        pqe.node = std::move(node);
        pqe.frequency = keyValuePair.second;
        pq.push(pqe);
    }

    // 3. Merge
    while (pq.size() > 1) {
        auto parent = std::make_shared<Node>();

        const PQEntry& e1 = pq.top(); 
        size_t f1 = e1.frequency;
        parent->childOne = e1.node;
        parent->childOne->parent = parent;
        pq.pop();

        const PQEntry& e2 = pq.top(); 
        size_t f2 = e2.frequency;
        parent->childTwo = e2.node;
        parent->childTwo->parent = parent;
        pq.pop();

        pq.push({ f1 + f2 ,std::move(parent) });
    }
    this->head = pq.top().node;
}

HuffmanTree::HuffmanTree(const std::uint8_t* raw, size_t byteCount) {
    std::shared_ptr<Node> node;
    const size_t totalBits = byteCount * 8;
    for (size_t bitPos = 0; bitPos < totalBits; bitPos++) {
        uint8_t byte = raw[bitPos >> 3];
        bool bit = byte & (1 << (bitPos & 0b111));
        if (bit) {
            uint16_t board = getBoardAtPos(raw, byteCount, bitPos + 1);
            bitPos += 15;
            auto newNode = std::make_shared<Node>();
            newNode->parent = node;
            newNode->value = board;

            if (node == nullptr) {
                node = newNode;
                this->head = node;
                continue;
            }

            if (node->childOne == nullptr) {
                node->childOne = newNode;
            } else {
                node->childTwo = newNode;
            }
            while (node != nullptr && node->childTwo != nullptr) {
                node = node->parent;
            }
            if (node == nullptr) return;
            
        } else {
            auto newNode = std::make_shared<Node>();
            newNode->parent = node;
            if (node == nullptr) {
                node = newNode;
                this->head = node;
                continue;
            }
            if (node->childOne == nullptr) {
                node->childOne = newNode;
            } else {
                node->childTwo = newNode;
            }
            node = newNode;
        }
    }
}

bool HuffmanTree::operator==(const HuffmanTree& other) const {
    return DFSE(this->head, other.head);
}

void HuffmanTree::serialize(ByteVector& raw) {
    if(this->head->childOne == nullptr) {
        return;
    }
    ByteVector serializeData;
    serializeData.reserve(raw.size());
    size_t newPos = 0;
    const size_t totalBits = raw.size() * 8;
    for (size_t bitPos = 0; bitPos + 15 <= totalBits; bitPos += 15) {
        uint16_t board = getBoardAtPos(raw.data(), raw.size(), bitPos);

        if (this->leafNodes.find(board) == this->leafNodes.end()) throw std::string("Leaf node do not exist");

        std::shared_ptr<Node> startNode = this->leafNodes[board];
        std::shared_ptr<Node> node = startNode;

        size_t depth = 0;
        while (node->parent != nullptr) {
            node = node->parent;
            ++depth;
        }

        node = startNode;
        if(serializeData.size() < ((newPos + depth + 7) >> 3)) serializeData.resize(((newPos + depth + 7) >> 3));
        for(int64_t i = newPos + depth - 1; i >= int64_t(newPos); i--) {
            if (node == node->parent->childTwo) {
                serializeData[i >> 3] |= 1 << (i & 0b111);
            }
            node = node->parent;
        }
        newPos += depth;
    }
    raw = std::move(serializeData);
}

ByteVector HuffmanTree::deserialization(const std::uint8_t* raw, size_t byteCount, size_t boardCount) {
    ByteVector deserializeData;
    if (byteCount <= 2) {
        writeBoardAtPos(deserializeData, 0, this->head->value, 15);
        return deserializeData;
    }
    deserializeData.reserve(byteCount*2u); // Approximate size
    size_t newPos = 0;

    std::shared_ptr<Node> node = this->head;
    const size_t totalBits = byteCount * 8;
    for (size_t bitPos = 0; bitPos < totalBits && boardCount; bitPos++) {
        if (raw[bitPos >> 3] & (1 << (bitPos & 0b111))) {
            node = node->childTwo;
        } else {
            node = node->childOne;
        }
        if (node->childOne == nullptr) {
            writeBoardAtPos(deserializeData, newPos, node->value, 15);
            newPos += 15;
            boardCount--;
            node = this->head;
            continue;
        }
    }
    return deserializeData;
}

ByteVector HuffmanTree::getHuffmanTree() {
    ByteVector data;
    std::shared_ptr<Node> node = this->head;
    size_t bitPos = 0;
    DFSC(node, data, bitPos);
    return data;
}
