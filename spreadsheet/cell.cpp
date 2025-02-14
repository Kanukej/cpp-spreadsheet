#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>

// Реализуйте следующие методы

Cell::Cell(std::string text, Position pos, SheetInterface& sheet) : pos_(pos), sheet_(sheet) {
    Clear();
    Set(text);
}

Cell::~Cell() {
    //
}

void Cell::ClearRefs() {
    for (const auto& pos : GetReferencedCells()) {
        auto* cell = reinterpret_cast<Cell*>(sheet_.GetCell(pos));
        if (cell != nullptr) {
            cell->DelReference(pos_);
        }
    }
}

void Cell::Invalidate() {
    has_value_ = false;
    for (const auto& pos : references_) {
        auto* cell = sheet_.GetCell(pos);
        if (cell != nullptr) {
            cell->Invalidate();
        }
    }
}

bool Cell::CheckDependencies(const std::vector<Position>& refs) const {
    for (const auto& pos : refs) {
        if (pos == pos_) {
            return false;
        }
        auto* cell = sheet_.GetCell(pos);
        if (cell != nullptr && !CheckDependencies(cell->GetReferencedCells())) {
            return false;
        }
    }
    return true;
}

void Cell::Set(std::string text) {
    Invalidate();
    ClearRefs();
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text[0] == ESCAPE_SIGN) {
        impl_ = std::make_unique<TextImpl>(text);
    } else if (text[0] == FORMULA_SIGN && text.size() > 1) {
        auto new_impl = std::make_unique<FormulaImpl>(text, sheet_);
        const auto refs = new_impl->GetReferencedCells();
        if (!CheckDependencies(refs)) {
            throw CircularDependencyException("circular dependency");
        }
        
        for (const auto& pos : refs) {
            auto* cell = sheet_.GetCell(pos);
            if (cell == nullptr) {
                sheet_.SetCell(pos, "");
            }
            sheet_.GetCell(pos)->AddReference(pos_);
        }
        
        impl_ = std::move(new_impl);
    } else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if (has_value_) {
        return cache_;
    }
    cache_ = impl_->GetValue();
    has_value_ = true;
    return cache_;
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !references_.empty();
}

bool Cell::Empty() const {
    return impl_->Empty();
}

void Cell::AddReference(const Position& pos) {
    references_.insert(pos);
}

void Cell::DelReference(const Position& pos) {
    references_.erase(pos);
}

Cell::Impl::Impl(const std::string& text) : raw_text_(text) {
    //
}

Cell::EmptyImpl::EmptyImpl() : Cell::Impl("") {
    //
}

CellInterface::Value Cell::EmptyImpl::GetValue() const {
    return raw_text_;
}

std::string Cell::EmptyImpl::GetText() const {
    return raw_text_;
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    return {};
}

bool Cell::EmptyImpl::Empty() const {
    return true;
}

Cell::TextImpl::TextImpl(const std::string& text) : Cell::Impl(text) {
    //
}

CellInterface::Value Cell::TextImpl::GetValue() const {
    if (!raw_text_.empty() && raw_text_[0] == ESCAPE_SIGN) {
        return raw_text_.substr(1);
    }
    return raw_text_;
}

std::string Cell::TextImpl::GetText() const {
    return raw_text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    return {};
}

bool Cell::TextImpl::Empty() const {
    return false;
}

Cell::FormulaImpl::FormulaImpl(const std::string& text, SheetInterface& sheet) : Cell::Impl(text), sheet_(sheet) {
    formula_ = ParseFormula(raw_text_.substr(1));
}

CellInterface::Value Cell::FormulaImpl::GetValue() const {
    auto result = formula_->Evaluate(sheet_);
    ValueVisitor ans;
    std::visit(ans, result);
    return ans.result;
}

std::string Cell::FormulaImpl::GetText() const {
    std::stringstream ss;
    ss << '=' << formula_->GetExpression();
    return ss.str();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

bool Cell::FormulaImpl::Empty() const {
    return false;
}
