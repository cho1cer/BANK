
#include "Person.h"
#include <iostream>
#include <fstream>
#include <functional> // 用于std::hash

// 构造函数：初始化个人属性
Person::Person(std::string &name, size_t age, std::string &gender,
               std::string &fingerprint, size_t socioeconomic_rank, bool is_alive)
    : name(name), age(age), gender(gender),
      hashed_fingerprint(std::hash<std::string>{}(fingerprint)),
      socioeconomic_rank(socioeconomic_rank), is_alive(is_alive)
{
    // 验证性别是否有效
    if (gender != "Male" && gender != "Female")
    {
        throw std::invalid_argument("Invalid gender: " + gender);
    }
    if (socioeconomic_rank < 1 || socioeconomic_rank > 10)
    {
        throw std::out_of_range("Socioeconomic rank is out of valid range (1-10): " + std::to_string(socioeconomic_rank));
    }
}
// 获取姓名
std::string Person::get_name() const
{
    return name;
}

// 获取年龄
size_t Person::get_age() const
{
    return age;
}

// 获取性别
std::string Person::get_gender() const
{
    return gender;
}

// 获取哈希后的指纹
size_t Person::get_hashed_fingerprint() const
{
    return hashed_fingerprint;
}

// 获取社会经济等级
size_t Person::get_socioeconomic_rank() const
{
    return socioeconomic_rank;
}

// 获取是否存活状态
bool Person::get_is_alive() const
{
    return is_alive;
}

// 设置年龄
bool Person::set_age(size_t age)
{
    // 直接设置年龄，因为 age 已经是非负的
    this->age = age;
    return true;
}

// 设置社会经济等级
// 设置社会经济等级
bool Person::set_socioeconomic_rank(size_t rank) {
    if (rank >= 1 && rank <= 10) {
        socioeconomic_rank = rank;
        return true;
    } else {
        throw std::out_of_range("Socioeconomic rank must be between 1 and 10.");
    }
}

// 设置是否存活
bool Person::set_is_alive(bool is_alive)
{
    this->is_alive = is_alive;
    return true;
}

// 实现三向比较运算符，比较个人的姓名
std::strong_ordering Person::operator<=>(const Person &other) const
{
    return name <=> other.name;
}

// 输出个人信息，支持写入文件
void Person::get_info(std::optional<std::string> file_name) const
{
    std::ostream *out = &std::cout;
    std::ofstream file;

    if (file_name)
    {
        file.open(*file_name);
        if (file.is_open())
        {
            out = &file;
        }
    }

    *out << "姓名: " << name << std::endl;
    *out << "年龄: " << age << std::endl;
    *out << "性别: " << gender << std::endl;
    *out << "社会经济等级: " << socioeconomic_rank << std::endl;
    *out << "是否存活: " << (is_alive ? "是" : "否") << std::endl;

    if (file.is_open())
    {
        file.close();
    }
}
