#ifndef MY_FUNCTION_HPP
#define MY_FUNCTION_HPP

template<typename Signature>
class Function;

template<typename Ret, typename... Args>
class Function<Ret(Args...)> {
public:
    Function() : callable(nullptr) {}
    template<typename Callable>
    Function(Callable c) : callable(new CallableHolder<Callable>(c)) {}
    Function(const Function& other) : callable(other.callable ? other.callable->clone() : nullptr) {}
    ~Function() { delete callable; }

    Function& operator=(const Function& other) {
        if (this != &other) {
            delete callable;
            callable = other.callable ? other.callable->clone() : nullptr;
        }
        return *this;
    }

    Ret operator()(Args... args) const {
        return callable->invoke(args...);
    }

    operator bool() const { return callable != nullptr; }

private:
    struct CallableBase {
        virtual ~CallableBase() {}
        virtual Ret invoke(Args... args) = 0;
        virtual CallableBase* clone() const = 0;
    };
    template<typename Callable>
    struct CallableHolder : CallableBase {
        Callable c;
        CallableHolder(const Callable& c_) : c(c_) {}
        Ret invoke(Args... args) override { return c(args...); }
        CallableBase* clone() const override { return new CallableHolder<Callable>(c); }
    };
    CallableBase* callable;
};

#endif