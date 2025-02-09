#pragma once
#include <panoc-alm/inner/directions/lbfgs.hpp>
#include <panoc-alm/inner/panoc.hpp>

#include <memory>

#include <pybind11/cast.h>
#include <pybind11/pytypes.h>
namespace py = pybind11;

namespace pa {

class PolymorphicPANOCDirectionBase
    : public std::enable_shared_from_this<PolymorphicPANOCDirectionBase> {
  public:
    virtual ~PolymorphicPANOCDirectionBase()         = default;
    virtual void initialize(const vec &x₀, const vec &x̂₀, const vec &p₀,
                            const vec &grad₀)        = 0;
    virtual bool update(const vec &xₖ, const vec &xₖ₊₁, const vec &pₖ,
                        const vec &pₖ₊₁, const vec &grad_new, const Box &C,
                        real_t γ_new)                = 0;
    virtual bool apply(const vec &xₖ, const vec &x̂ₖ, const vec &pₖ, real_t γ,
                       vec &qₖ)                      = 0;
    virtual void changed_γ(real_t γₖ, real_t old_γₖ) = 0;
    virtual void reset()                             = 0;
    virtual std::string get_name() const             = 0;
    vec apply_ret(const vec &xₖ, const vec &x̂ₖ, const vec &pₖ, real_t γ) {
        vec qₖ(pₖ.size());
        apply(xₖ, x̂ₖ, pₖ, γ, qₖ);
        return qₖ;
    }
    virtual py::object get_params() const = 0;
};

class PolymorphicPANOCDirectionTrampoline
    : public PolymorphicPANOCDirectionBase {
  public:
    void initialize(const vec &x₀, const vec &x̂₀, const vec &p₀,
                    const vec &grad₀) override {
        PYBIND11_OVERRIDE_PURE(void, PolymorphicPANOCDirectionBase, initialize,
                               x₀, x̂₀, p₀, grad₀);
    }
    bool update(const vec &xₖ, const vec &xₖ₊₁, const vec &pₖ, const vec &pₖ₊₁,
                const vec &grad_new, const Box &C, real_t γ_new) override {
        PYBIND11_OVERRIDE_PURE(bool, PolymorphicPANOCDirectionBase, update, xₖ,
                               xₖ₊₁, pₖ, pₖ₊₁, grad_new, C, γ_new);
    }
    bool apply(const vec &xₖ, const vec &x̂ₖ, const vec &pₖ, real_t γ,
               vec &qₖ) override {
        PYBIND11_OVERRIDE_PURE(bool, PolymorphicPANOCDirectionBase, apply, xₖ,
                               x̂ₖ, pₖ, γ, qₖ);
    }
    void changed_γ(real_t γₖ, real_t old_γₖ) override {
        PYBIND11_OVERRIDE_PURE(void, PolymorphicPANOCDirectionBase, changed_γ,
                               γₖ, old_γₖ);
    }
    void reset() override {
        PYBIND11_OVERRIDE_PURE(void, PolymorphicPANOCDirectionBase, reset, );
    }
    std::string get_name() const override {
        PYBIND11_OVERRIDE_PURE(std::string, PolymorphicPANOCDirectionBase,
                               get_name, );
    }
    py::object get_params() const override {
        PYBIND11_OVERRIDE_PURE(py::object, PolymorphicPANOCDirectionBase,
                               get_params, );
    }
};

template <>
struct PANOCDirection<PolymorphicPANOCDirectionBase> {
    using DirectionPtr = std::shared_ptr<PolymorphicPANOCDirectionBase>;
    DirectionPtr direction;

    PANOCDirection(const DirectionPtr &direction) : direction(direction) {}
    PANOCDirection(DirectionPtr &&direction)
        : direction(std::move(direction)) {}

    void initialize(const vec &x₀, const vec &x̂₀, const vec &p₀,
                    const vec &grad₀) {
        direction->initialize(x₀, x̂₀, p₀, grad₀);
    }
    bool update(const vec &xₖ, const vec &xₖ₊₁, const vec &pₖ, const vec &pₖ₊₁,
                const vec &grad_new, const Box &C, real_t γ_new) {
        return direction->update(xₖ, xₖ₊₁, pₖ, pₖ₊₁, grad_new, C, γ_new);
    }
    bool apply(const vec &xₖ, const vec &x̂ₖ, const vec &pₖ, real_t γ, vec &qₖ) {
        return direction->apply(xₖ, x̂ₖ, pₖ, γ, qₖ);
    }
    void changed_γ(real_t γₖ, real_t old_γₖ) {
        direction->changed_γ(γₖ, old_γₖ);
    }
    void reset() { direction->reset(); }
    std::string get_name() const { return direction->get_name(); }
};

template <class DirectionProviderT>
class PolymorphicPANOCDirection : public PolymorphicPANOCDirectionBase {

  public:
    using DirectionProvider = DirectionProviderT;

    PolymorphicPANOCDirection(DirectionProvider &&direction)
        : direction_provider(std::forward<DirectionProvider>(direction)) {}
    PolymorphicPANOCDirection(const DirectionProvider &direction)
        : direction_provider(direction) {}

    void initialize(const vec &x₀, const vec &x̂₀, const vec &p₀,
                    const vec &grad₀) override {
        direction_provider.initialize(x₀, x̂₀, p₀, grad₀);
    }
    bool update(const vec &xₖ, const vec &xₖ₊₁, const vec &pₖ, const vec &pₖ₊₁,
                const vec &grad_new, const Box &C, real_t γ_new) override {
        return direction_provider.update(xₖ, xₖ₊₁, pₖ, pₖ₊₁, grad_new, C,
                                         γ_new);
    }
    bool apply(const vec &xₖ, const vec &x̂ₖ, const vec &pₖ, real_t γ,
               vec &qₖ) override {
        return direction_provider.apply(xₖ, x̂ₖ, pₖ, γ, qₖ);
    }
    void changed_γ(real_t γₖ, real_t old_γₖ) override {
        direction_provider.changed_γ(γₖ, old_γₖ);
    }
    void reset() override { direction_provider.reset(); }
    std::string get_name() const override {
        return direction_provider.get_name();
    }
    py::object get_params() const override {
        return py::cast(direction_provider.get_params());
    }

  private:
    PANOCDirection<DirectionProvider> direction_provider;
};

using PolymorphicLBFGSDirection = PolymorphicPANOCDirection<LBFGS>;

} // namespace pa
