#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Cell : public CellInterface {
public:
    explicit Cell(std::string text, Position pos, SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    void AddReference(const Position& pos) override;
    void DelReference(const Position& pos) override;
    void Invalidate() override;

private:
    class Impl {
    public:
        Impl(const std::string& text);
        virtual CellInterface::Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual bool Empty() const = 0;
    protected:
        const std::string raw_text_;
    };
    class EmptyImpl : public Impl {
    public:
        EmptyImpl();
        CellInterface::Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool Empty() const override;
    };
    class TextImpl : public Impl {
    public:
        TextImpl(const std::string& text);
        CellInterface::Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool Empty() const override;
    };
    class FormulaImpl : public Impl {
    public:
        FormulaImpl(const std::string& text, SheetInterface& sheet);
        CellInterface::Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool Empty() const override;
    private:
        struct ValueVisitor {
            CellInterface::Value result;
            void operator()(const double& val) {
                result = val;
            }
            void operator()(const FormulaError& val) {
                result = val;
            }
        };
        SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> formula_;
    };

    void ClearRefs();
    bool Empty() const;
    bool CheckDependencies(const std::vector<Position>& refs) const;

    std::unordered_set<Position, PositionHash> references_;
    mutable bool has_value_ = false;
    const Position pos_;
    SheetInterface& sheet_;
    mutable Value cache_;
    std::unique_ptr<Impl> impl_;
};
