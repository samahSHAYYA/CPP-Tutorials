/**
 * @file test_type_traits_extensions.cpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This file contains all tests related to type trait extensions defined
 *        in header-only type_traits_extensions.hpp file.
 */

#include <gtest/gtest.h>

#include "tree/type_traits_extensions.hpp"

// Testing HasToStringMethod / HasToStringMethod_v
struct MyClassWithToString {
    std::string toString() const { return "MyClassWithToString"; }
};

struct MyClassWithToStringButNotSameSignature {
    std::string toString(int i) const { return std::to_string(i); }
};

struct MyClassWithoutToString {};

TEST(TypeTraitsExtensionsTest, HasToStringMethodTest) {
    std::cout << std::boolalpha;
    std::cout << "HasToStringMethod_v<MyClassWithToString>: " << HasToStringMethod_v<MyClassWithToString> << std::endl;
    EXPECT_TRUE(HasToStringMethod_v<MyClassWithToString>);
    EXPECT_FALSE(HasToStringMethod_v<MyClassWithToStringButNotSameSignature>);
    EXPECT_FALSE(HasToStringMethod_v<MyClassWithoutToString>);
}

// Testing for HasLessThanOrEqualOperator_v, HasEqualityOperator_v , IsComparable
struct MyComparableStruct {
    bool operator<=(const MyComparableStruct&) const { return true; }
    bool operator==(const MyComparableStruct&) const { return true; }
};

struct MyNonComparableStruct {
    bool operator==(const MyNonComparableStruct&) const { return true; }
};

TEST(TypeTraitsExtensionsTest, IsComparableTest) {
    EXPECT_TRUE(HasLessThanOrEqualOperator_v<MyComparableStruct>);
    EXPECT_TRUE(HasEqualityOperator_v<MyComparableStruct>);

    EXPECT_FALSE(HasLessThanOrEqualOperator_v<MyNonComparableStruct>);
    EXPECT_TRUE(HasEqualityOperator_v<MyNonComparableStruct>);

    EXPECT_TRUE(IsComparable<MyComparableStruct>);
    EXPECT_FALSE(IsComparable<MyNonComparableStruct>);
}

// Testing for IsConvertibleToString
struct MyNonConvertibleToStringStruct {};

TEST(TypeTraitsExtensionsTest, IsConvertibleToStringTest) {
    EXPECT_TRUE(IsConvertibleToString<MyClassWithToString>);
    EXPECT_FALSE(IsConvertibleToString<MyClassWithToStringButNotSameSignature>);
    EXPECT_FALSE(IsConvertibleToString<MyNonConvertibleToStringStruct>);
}

// Testing AreConvertibleToStrings
TEST(TypeTraitsExtensionsTest, AreConvertibleToStringsTest) {
    /**
     * Below we use additional parenthesis inside EXPECT_TRUE to allow proper
     * macro expansion.
     */
    EXPECT_TRUE((AreConvertibleToStrings<int,
                                         double,
                                         std::string,
                                         MyClassWithToString>));

    EXPECT_FALSE((AreConvertibleToStrings<int,
                                          double,
                                          MyClassWithToString,
                                          MyNonConvertibleToStringStruct>));
}

// Testing HasEqualityOperator / HasEqualityOperator_v
struct MyStructWithEqualityOperator {
    bool operator==(const MyStructWithEqualityOperator&) const { return true; }
};

struct MyStructWithoutEqualityOperator {};

TEST(TypeTraitsExtensionsTest, HasEqualityOperatorTest) {
    EXPECT_TRUE(HasEqualityOperator_v<MyStructWithEqualityOperator>);
    EXPECT_FALSE(HasEqualityOperator_v<MyStructWithoutEqualityOperator>);
}

// Testing HasSerializeMethod_v, HasDeserializeMethod_v, IsSerializable
struct MySerializableStruct {
    bool serialize(std::ostream&) const { return true; }
    static MySerializableStruct deserialize(std::ifstream&) { return MySerializableStruct(); }
};

struct MyNonSerializableStruct {};

TEST(TypeTraitsExtensionsTest, IsSerializableTest) {
    EXPECT_TRUE(HasSerializeMethod_v<MySerializableStruct>);
    EXPECT_TRUE(HasDeserializeMethod_v<MySerializableStruct>);
    EXPECT_TRUE(IsSerializable<MySerializableStruct>);

    EXPECT_FALSE(HasSerializeMethod_v<MyNonSerializableStruct>);
    EXPECT_FALSE(HasDeserializeMethod_v<MyNonSerializableStruct>);
    EXPECT_FALSE(IsSerializable<MyNonSerializableStruct>);
}

// Testing AreSerializable
TEST(TypeTraitsExtensionsTest, AresSerializableTest) {
    EXPECT_TRUE((AreSerializable<int,
                                 double,
                                 std::string,
                                 MySerializableStruct>));

    EXPECT_FALSE((AreSerializable<int,
                                  double,
                                  MySerializableStruct,
                                  MyNonSerializableStruct>));
}

int main(int argc, char** argv) {
    /**
     * Note that InitGoogleTest consumes all arguments that recognizes and
     * keeps the rest.
     */
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
