
#include "Dialog.h"
#include <functional>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <tuple>
#include <type_traits>

// Базовый класс для параметров диалога
struct DialogParamBase {
    std::string name;
    std::string displayName;

    DialogParamBase(const std::string& name, const std::string& displayName)
        : name(name), displayName(displayName) {}

    virtual ~DialogParamBase() = default;
    virtual void addToDialog(TAbstractDialog* dialog) = 0;
    virtual void* getValuePtr() = 0;
};

// Шаблонный класс для параметров
template<typename T>
struct DialogParam : DialogParamBase {
    T value;
    T defaultValue;

    DialogParam(const std::string& name, const std::string& displayName, T defaultValue)
        : DialogParamBase(name, displayName), value(defaultValue), defaultValue(defaultValue) {}

    void addToDialog(TAbstractDialog* dialog) override {
        dialog->AddInput<T>(displayName, value);
    }

    void* getValuePtr() override {
        return &value;
    }
};

// Вспомогательные traits для определения сигнатуры функции
template<typename T>
struct function_traits;

// Для std::function
template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};

// Для обычных указателей на функции
template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};

// Для лямбд и функциональных объектов
template<typename Functor>
struct function_traits {
private:
    using call_type = function_traits<decltype(&Functor::operator())>;
public:
    using result_type = typename call_type::result_type;
    using args_tuple = typename call_type::args_tuple;
    static constexpr size_t arity = call_type::arity;
};

// Для константных operator()
template<typename ClassType, typename R, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};

// Для неконстантных operator()
template<typename ClassType, typename R, typename... Args>
struct function_traits<R(ClassType::*)(Args...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};

// Основной класс-обертка для функций с возвращаемым значением
template<typename Func>
class FunctionDialogWrapper {
private:
    Func func_;
    std::vector<std::unique_ptr<DialogParamBase>> params_;

    using traits = function_traits<Func>;
    using result_type = typename traits::result_type;

public:
    FunctionDialogWrapper(Func func) : func_(func) {}

    // Метод для добавления параметра
    template<typename T>
    FunctionDialogWrapper& addParam(const std::string& name, const std::string& displayName, T defaultValue) {
        params_.push_back(std::make_unique<DialogParam<T>>(name, displayName, defaultValue));
        return *this;
    }

    // Запуск диалога и выполнение функции с возвратом optional результата
	result_type execute() {
        if (params_.empty()) {
            return callFunc(std::make_index_sequence<traits::arity>{});
        }

        TAbstractDialog* dialog = new TAbstractDialog(nullptr);

        // Добавляем все параметры в диалог
        for (auto& param : params_) {
            param->addToDialog(dialog);
        }

        result_type result;
        if (dialog->Execute() && dialog->ContinuePressed) {
            // Вызываем функцию с параметрами
            result = callFunc(std::make_index_sequence<traits::arity>{});
        }

        delete dialog;
        return result;
    }

    // Альтернативная версия с передачей результата по ссылке
    bool execute(result_type& result) {
        if (params_.empty()) {
            result = callFunc(std::make_index_sequence<traits::arity>{});
            return true;
        }

        TAbstractDialog* dialog = new TAbstractDialog(nullptr);

        for (auto& param : params_) {
            param->addToDialog(dialog);
        }

        bool success = false;
        if (dialog->Execute() && dialog->ContinuePressed) {
            result = callFunc(std::make_index_sequence<traits::arity>{});
            success = true;
        }

        delete dialog;
        return success;
    }

    // Версия только для запуска диалога (для void функций)
    template<typename R = result_type>
    typename std::enable_if<std::is_same<R, void>::value, bool>::type
    execute() {
        if (params_.empty()) {
            callFunc(std::make_index_sequence<traits::arity>{});
            return true;
        }

        TAbstractDialog* dialog = new TAbstractDialog(nullptr);

        for (auto& param : params_) {
            param->addToDialog(dialog);
        }

        bool success = false;
        if (dialog->Execute() && dialog->ContinuePressed) {
            callFunc(std::make_index_sequence<traits::arity>{});
            success = true;
        }

        delete dialog;
        return success;
    }

private:
    // Вспомогательный метод для вызова функции с параметрами
    template<std::size_t... I>
    result_type callFunc(std::index_sequence<I...>) {
        return func_((*static_cast<typename std::tuple_element<I, typename traits::args_tuple>::type*>(
            params_[I]->getValuePtr()))...);
    }
};

// Вспомогательная функция для создания обертки (использует auto для вывода типа)
template<typename Func>
auto createDialogWrapper(Func func) {
    return FunctionDialogWrapper<Func>(func);
}

// Явная специализация для std::function (для обратной совместимости)
template<typename R, typename... Args>
auto createDialogWrapper(std::function<R(Args...)> func) {
    return FunctionDialogWrapper<std::function<R(Args...)>>(func);
}
// Макрос для упрощения создания обертки (опционально)
#define WRAP_WITH_DIALOG(func, ...) createDialogWrapper(std::function(func)).addParam(__VA_ARGS__)
