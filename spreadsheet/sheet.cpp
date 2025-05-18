#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::Sheet() {
}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!CheckPosition(pos)) {
        throw InvalidPositionException("Invalid cell position");
    }
    cells_[pos] = std::make_unique<Cell>(text, pos, *this);
    rows_idx_[pos.row].insert(pos.col);
    cols_idx_[pos.col].insert(pos.row);
    if (pos.row >= size_.rows) {
        size_.rows = pos.row + 1;
    }
    if (pos.col >= size_.cols) {
        size_.cols = pos.col + 1;
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!CheckPosition(pos)) {
        throw InvalidPositionException("Invalid cell position");
    }
    const auto cell = cells_.find(pos);
    if (cell != nullptr) {
        return cell->second.get();
    }
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!CheckPosition(pos)) {
        throw InvalidPositionException("Invalid cell position");
    }
    auto cell = cells_.find(pos);
    if (cell != nullptr) {
        return cell->second.get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!CheckPosition(pos)) {
        throw InvalidPositionException("Invalid cell position");
    }
    auto cell = cells_.find(pos);
    if (cell != nullptr) {
        cells_.erase(cell);
        rows_idx_[pos.row].erase(pos.col);
        cols_idx_[pos.col].erase(pos.row);
        UpdateSize();
    }
}

void Sheet::UpdateSize() {
    while (size_.rows > 0 && rows_idx_[size_.rows - 1].empty()) {
        rows_idx_.erase(size_.rows - 1);
        --size_.rows;
    }
    while (size_.cols > 0 && cols_idx_[size_.cols - 1].empty()) {
        cols_idx_.erase(size_.cols - 1);
        --size_.cols;
    }
}

bool Sheet::CheckPosition(const Position& pos) const {
    return pos.IsValid();
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

template <typename Func>
void Sheet::Print(std::ostream& output, Func pred) const {
    for (int row_id = 0; row_id < size_.rows; ++row_id) {
        bool first_col = true;
        for (int col_id = 0; col_id < size_.cols; ++col_id) {
            const auto cell = cells_.find(Position {row_id, col_id});
            if (first_col) {
                first_col = false;
            } else {
                output << '\t';
            }
            if (cell != nullptr) {
                pred(output, *cell->second);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    Print(output, [&output](std::ostream& os, const CellInterface& cell) {
        std::visit(ValueVisitor { output }, cell.GetValue());
    });
}

void Sheet::PrintTexts(std::ostream& output) const {
    Print(output, [](std::ostream& os, const CellInterface& cell) {
        os << cell.GetText();
    });

}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
