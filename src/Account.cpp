#include "Account.h"
#include "Person.h"
#include "Bank.h"
#include <iostream>
#include <fstream>
#include <random>
#include <compare>
#include <cmath>
// 生成随机账号号码
std::string generate_random_account_number()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);
    std::string account_number;
    for (int i = 0; i < 16; ++i)
    {
        account_number += std::to_string(dis(gen));
    }
    return account_number;
}

// 生成随机CVV2
std::string generate_random_CVV2()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    return std::to_string(dis(gen));
}

// 生成随机到期日
// 生成随机到期日
std::string generate_random_exp_date() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 12);
    int month = dis(gen);
    int year = 20 + dis(gen) % 100; // 确保年份是两位数
    char buffer[8]; // 增加缓冲区大小以确保有足够的空间
    std::snprintf(buffer, sizeof(buffer), "%02d-%02d", year, month);
    return std::string(buffer);
}
// 构造函数：初始化账户所有者、银行、密码和相关信息
Account::Account(const Person *const owner, const Bank *const bank, std::string &password)
    : owner(const_cast<Person *>(owner)), bank(bank),
      account_number(generate_random_account_number()),
      balance(0.0), account_status(true), CVV2(generate_random_CVV2()),
      password(password), exp_date(generate_random_exp_date())
{
    // 在这里可以添加生成随机账户号、CVV2、到期日等逻辑
}

// 获取账户所有者信息
const Person *Account::get_owner() const
{
    return owner;
}

// 获取账户余额
double Account::get_balance() const
{
    return balance;
}

// 获取账户号码
std::string Account::get_account_number() const
{
    return account_number;
}

// 获取账户状态（true为活跃，false为关闭）
bool Account::get_status() const
{
    return account_status;
}
std::string Account::get_CVV2(std::string& owner_fingerprint) const {
    if (owner->get_hashed_fingerprint() == std::hash<std::string>{}(owner_fingerprint)) {
        return CVV2; // 指纹验证成功，返回CVV2
    } else {
        throw std::runtime_error("指纹验证失败"); // 指纹验证失败，抛出异常
    }
}

// 获取密码，需验证所有者指纹
std::string Account::get_password(std::string &owner_fingerprint) const
{
    if (owner->get_hashed_fingerprint() == std::hash<std::string>{}(owner_fingerprint))
    {
        return password;
    }
    else
    {
        throw std::runtime_error("指纹验证失败"); 
    }
}

// 获取到期日，需验证所有者指纹
std::string Account::get_exp_date(std::string &owner_fingerprint) const
{
    if (owner->get_hashed_fingerprint() == std::hash<std::string>{}(owner_fingerprint))
    {
        return exp_date;
    }
    else
    {
        throw std::runtime_error("指纹验证失败"); 
    }
}

// 设置新密码，需验证所有者指纹
bool Account::set_password(std::string &new_password, std::string &owner_fingerprint)
{
    if (owner->get_hashed_fingerprint() == std::hash<std::string>{}(owner_fingerprint))
    {
        password = new_password;
        return true;
    }
    else
    {
        return false;
    }
}

// 实现三向比较运算符，比较账户号
std::strong_ordering Account::operator<=>(const Account &other) const
{
    return account_number <=> other.account_number;
}

// 输出账户信息，支持写入到文件
void Account::get_info(std::optional<std::string> file_name) const
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

    *out << "账户所有者: " << owner->get_name() << std::endl;
    *out << "账户号: " << account_number << std::endl;
    *out << "余额: " << balance << std::endl;
    *out << "状态: " << (account_status ? "活跃" : "关闭") << std::endl;

    if (file.is_open())
    {
        file.close();
    }
}