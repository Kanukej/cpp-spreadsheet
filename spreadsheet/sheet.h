#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <iostream>

class Sheet : public SheetInterface {
public:
    Sheet();
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;


private:
    struct ValueVisitor {
        std::ostream& out;
        void operator()(const std::string& val) {
            out << val;
        }
        void operator()(const double& val) {
            out << val;
        }
        void operator()(const FormulaError& val) {
            out << val;
        }
    };

    void UpdateSize();
    bool CheckPosition(const Position& pos) const;
    
    template <typename Func>
    void Print(std::ostream& output, Func pred) const;
    
    std::unordered_map<int, std::unordered_set<int>> cols_idx_;
    std::unordered_map<int, std::unordered_set<int>> rows_idx_;
    Size size_ {0, 0};

    std::unordered_map<Position, std::unique_ptr<CellInterface>, PositionHash> cells_;
};
