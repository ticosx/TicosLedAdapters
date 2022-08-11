#ifndef __COLOR_PATTERN_H__
#define __COLOR_PATTERN_H__


struct TColor {
    constexpr TColor()
        : r(0)
        , g(0)
        , b(0) {}
    constexpr TColor(uint8_t cr, uint8_t cg, uint8_t cb)
        : r(cr)
        , g(cg)
        , b(cb) {}
    uint8_t r, g, b;
};

class LedPatternNode {
private:
    TColor _color;
    LedPatternNode* _next = nullptr;
public:
    LedPatternNode() {};
    LedPatternNode(TColor color) {
      setColor(color);
    };

    void setColor(TColor color) {
      _color = color;
    };
    void setNext(LedPatternNode* next) {
      _next = next;
    };

    TColor color() const {
        return _color;
    };

    LedPatternNode* next() const {
        return _next;
    };

    LedPatternNode* nextLooped(LedPatternNode* first) const {
        return (_next == nullptr) ? first : _next;
    };
};

class LedPattern {
private:
    LedPatternNode* _first = nullptr;
    LedPatternNode* _last = nullptr;
public:
    void add(LedPatternNode* node) {
        if (_first == nullptr) {
            _first = node;
        }
        if (_last == nullptr) {
            _last = node;
        } else {
            // Link nodes
            _last->setNext(node);
            // Set node as _last
            _last = node;
        }
    };

    LedPatternNode* first() const {
        return _first;
    };

    LedPatternNode* last() const {
        return _last;
    };
};

#endif
