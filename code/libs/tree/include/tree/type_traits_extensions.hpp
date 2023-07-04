/**
 * @file type_traits_extensions.hpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This is header-only file containing type traits extensions to check
 *        whether a type has given public / protected / private method (static
 *        or non-static), operator, assign-operator, constructors, etc.
 */

#ifndef TYPE_TRAITS_EXTENSIONS_HPP
#define TYPE_TRAITS_EXTENSIONS_HPP

#include <type_traits>
#include <string>
#include <fstream>

/**
 * REMARK:
 *
 * This is a header-only file and does not require separate source file or
 * explicit implementations (explicit empty implementation). The traits
 * provided here are used solely for compile-time checks and do not require
 * runtime functionality.
 */

/**
 * @brief Type trait to check if a class has a member function named
 *        `toString` with the following signature:
 *         std::string (T::*)() const.
 *
 * @tparam T The type for which to check if `toString` method exists.
 *
 * @note If struct inherits from another class / struct, default inheritance
 *       access is public, unlike one class inherits from another class / struct
 *       in which default inheritance access is private.
 *
 * @note The second unnamed template typename defaulted to void is there to
 *       allow specialization.
 *
 * @details
 *
 * How it works?
 *
 * 1) The primary template of HasToStringMethod is defined with two template
 *    parameters:
 *    - `T`: The type we want to check for the existence of the toString method.
 *    - `typename = void`: A default template parameter, which is not used in
 *       the primary template (but useful for specialization).
 *
 * 2) The primary template provides a member named `value`, and it is set to
 *    `std::false_type` by default, indicating that the `toString` method is
 *    not available. This is done by means of inheriting `std::false_type`.
 *
 * 3) The partial specialization of HasToStringMethod is used to perform the
 *    actual check for the existence of the `toString` method. It uses
 *    `std::void_t` to check the validity of
 *    `decltype(std::declval<const T>().toString())`.
 *
 * 4) `std::void_t` is a utility from C++17 that doesn't actually return a
 *     value. It is used here to perform a "dependent type check." If the
 *     expression `decltype(std::declval<const T>().toString())` is valid,
 *     `std::void_t` will return void. Otherwise, it will cause a substitution
 *     failure.
 *
 * 5) The partial specialization is enabled only if the type
 *    `decltype(std::declval<const T>().toString())` is valid, which means that
 *    the `toString` method exists and is callable with `const T` as the object.
 *
 * 6) When the partial specialization is enabled (i.e., `T` has a `toString`
 *    method), `std::is_invocable_r` is used to check if `T::toString` is
 *    callable and returns `std::string` when called with `const T`. If the
 *    condition is true, the value member is set to std::true_type.
 *
 * So, in summary, this type trait checks if a class `T` has a member function
 * `toString` that can be invoked with `const T` and returns `std::string`.
 * If the `toString` method is present and satisfies the expected signature,
 * `HasToStringMethod<T>::value` will be true. Otherwise, it will be false. *
 */
template<typename T, typename = void>
struct HasToStringMethod : std::false_type {};

template<typename T>
struct HasToStringMethod<T, std::void_t<decltype(std::declval<const T>().toString())>> :
       std::is_invocable_r<std::string, decltype(&T::toString), const T> {};

// Creating a convenience alias
template<typename T>
constexpr bool HasToStringMethod_v = HasToStringMethod<T>::value;

/**
 * @brief Type trait to check if a class has a less-than or equal `<=` operator.
 *
 * @tparam T The type for which to check if `<=` operator exists.
 *
 * @note If T is fundamental type, the `value` will be true.
 */
template<typename T>
struct HasLessThanOrEqualOperator {
    private:
        template <typename U, typename = decltype(std::declval<U>() <= std::declval<U>())>
        static std::true_type test(int);

        template <typename>
        static std::false_type test(...);

    public:
        static constexpr bool value = (std::is_same_v<decltype(test<T>(0)), std::true_type> ||
                                       std::is_fundamental_v<T>);
};

template<typename T>
constexpr bool HasLessThanOrEqualOperator_v = HasLessThanOrEqualOperator<T>::value;

/**
 * @brief Type trait to check if a class has an equality `==` operator.
 *
 * @tparam T The type for which to check if `==` operator exists.
 *
 * @note If T is fundamental type, `value` will be true.
 */
template <typename T>
struct HasEqualityOperator {
    private:
        template <typename U, typename = decltype(std::declval<U>() == std::declval<U>())>
        static std::true_type test(int);

        template <typename>
        static std::false_type test(...);

    public:
        static constexpr bool value = (std::is_same_v<decltype(test<T>(0)), std::true_type> ||
                                    std::is_fundamental_v<T>);
};

template<typename T>
constexpr bool HasEqualityOperator_v = HasEqualityOperator<T>::value;

/**
 * REMARK:
 *
 * Below we provide some type traits needed for NodeType and Tree functionality.
 */

/**
 * @brief Type trait to check if a type is comparable using the less-than (<)
 *        and equality (==) operators.
 *
 * @tparam T The type to check.
 *
 * @note The less-than and equality operators must have the following
 *       signatures:
 *       - `bool T::operator<(const T&) const'
 *       - `bool T::operator==(const T&) const`
 *
 * @example Example:
 * @code{.cpp}
 * struct MyStruct {
 *     bool operator<(const MyStruct&) const;
 *     bool operator==(const MyStruct&) const;
 * };
 *
 * bool IsComparable = IsComparable<MyStruct>;  // true
 * @code
 *
 * @note This type trait relies on the `HasOperatorWithExpectedSignature_v` type
 *       trait to check for the presence of the less-than and equality
 *       operators with the expected return type.
 */
template<typename T>
constexpr bool IsComparable = (HasLessThanOrEqualOperator_v<T> &&
                               HasEqualityOperator_v<T>);

template<typename... Args>
constexpr bool AreComparable = (IsComparable<Args> && ...);

/**
 * @brief Type trait to determine if a type is convertible to a string. This
 *        trait evaluates whether a given type can be converted to a string
 *        representation. The type is considered convertible to a string if it
 *        satisfies any of the following conditions:
 *        - It is a void type.
 *        - It is a fundamental type.
 *        - It is the std::string type or a derived type of std::string.
 *        - It has a ToString method defined.
 *
 * @tparam T The type to check for string convertibility.
 */
template<typename T>
constexpr bool IsConvertibleToString = (std::is_void_v<T> ||
                                        std::is_fundamental_v<T> ||
                                        std::is_same_v<T, std::string> ||
                                        std::is_base_of_v<std::string, T> ||
                                        HasToStringMethod_v<T>);

/**
 * @brief Type trait that determines if all passed template argument types
 *        are convertible to strings.
 *
 * @tparam Args The variadic type arguments to check for convertibility to
 *              strings.
 */
template<typename... Args>
constexpr bool AreConvertibleToStrings = (IsConvertibleToString<Args> && ...);

/**
 * @brief Type trait to check if a class has a member function named
 *        `serialize` with the following signature:
 *         bool (T::*)(std::ofstream&) const.
 *
 * @tparam T The type for which to check if `serialize` method exists.
 */
template<typename T, typename = void>
struct HasSerializeMethod : std::false_type {};

template<typename T>
struct HasSerializeMethod<T, std::void_t<decltype(std::declval<const T>().serialize(std::declval<std::ofstream&>()))>> :
       std::is_invocable_r<bool, decltype(&T::serialize), const T, std::ofstream&> {};

template<typename T>
constexpr bool HasSerializeMethod_v = HasSerializeMethod<T>::value;

/**
 * @brief Type trait to check if a class has a static member function named
 *        `deserialize` with the following signature:
 *         static T (T::*)(std::ifstream&).
 *
 * @tparam T The type for which to check if `serialize` method exists.
 */
template<typename T, typename = void>
struct HasDeserializeMethod : std::false_type {};

/**
 * REMARK:
 *
 * In std::is_invocable_r part following `decltype(&T::deserialize)`, we do
 * not pass `T` as `deserialize` is static method. We only pass
 * `std::ifstream&` as argument. This is what differentiates it from
 * `serialize`, which is non-static method.
 */
template<typename T>
struct HasDeserializeMethod<T, std::void_t<decltype(T::deserialize(std::declval<std::ifstream&>()))>> :
       std::is_invocable_r<T, decltype(&T::deserialize), std::ifstream&> {};

template<typename T>
constexpr bool HasDeserializeMethod_v = HasDeserializeMethod<T>::value;

/**
 * @brief Type trait to check if a type is serializable. This type `T` is
 *        serializable, meaning it can be converted to and from a serialized
 *        representation, if it satisfies one of the following conditions:
 *        - It is a void type.
 *        - It is a fundamental type.
 *        - It is a std::string type.
 *        - It has both a serialize method and a deserialize method.
 *
 * @tparam T The type to check.
 *
 * @note The serialize method must have the following signature:
 *       - `bool T::serialize(std::ostream&)`
 *
 * @note The deserialize method must have the following signature:
 *       - `bool T::deserialize(std::istream&)`
 *
 * @example Example:
 * @code{.cpp}
 * struct MyStruct {
 *     bool serialize(std::ostream&);
 *     static MyStruct deserialize(std::istream&);
 * };
 *
 * bool IsSerializable = IsSerializable<MyStruct>;  // true
 * @code
 *
 * @note This type trait relies on the `HasSerializeMethod` and
 *       `HasDeserializeMethod` type traits to check for the presence of the
 *       serialize and deserialize methods with the expected signatures.
 */
template<typename T>
constexpr bool IsSerializable = (std::is_void_v<T> ||
                                 std::is_fundamental_v<T> ||
                                 std::is_same_v<T, std::string> ||
                                 std::is_base_of_v<std::string, T> ||
                                 (HasSerializeMethod_v<T> &&
                                  HasDeserializeMethod_v<T>));

/**
 * @brief Type trait that determines if all passed template argument types
 *        are serializable.
 *
 * @tparam Args The variadic type arguments to check if they are serializable.
 */
template<typename... Args>
constexpr bool AreSerializable = (IsSerializable<Args> && ...);

#endif
