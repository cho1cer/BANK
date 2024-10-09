#include "Bank.h"
#include "Account.h"
#include "Person.h"
#include <iostream>
#include <fstream>
#include <functional> // 用于std::hash
#include <cmath>
#include <memory>
// 构造函数：初始化银行名称和指纹哈希值
Bank::Bank(const std::string &name, const std::string &bank_fingerprint)
    : bank_name(name),
      hashed_bank_fingerprint(std::hash<std::string>{}(bank_fingerprint)),
      bank_total_balance(0.0),
      bank_total_loan(0.0)
{
    // 其他必要的初始化
}

// 析构函数
Bank::~Bank() {

}



// 创建账户，验证指纹后返回账户指针
Account* Bank::create_account(Person &owner, const std::string &owner_fingerprint, std::string password) {
    if (owner.get_hashed_fingerprint() == std::hash<std::string>{}(owner_fingerprint)) {
        // 动态分配新账户
        Account* new_account = new Account(&owner, this, password);

        // 添加账户到银行账户列表
        bank_accounts.push_back(new_account);

        // 映射账户到客户
        account_2_customer[new_account] = &owner;

        // 将账户添加到客户的账户列表
        customer_2_accounts[&owner].push_back(new_account);

        // 检查客户是否已经存在
        if (std::find(bank_customers.begin(), bank_customers.end(), &owner) == bank_customers.end()) {
            bank_customers.push_back(&owner);
        }

        return new_account;
    }
    return nullptr;
}
bool Bank::delete_account(Account &account, const std::string &owner_fingerprint) {
    // 获取账户所有者
    Person *owner = account_2_customer[&account];
    if (!owner) {
        // 如果找不到所有者，返回 false
        return false;
    }

    // 验证指纹
    size_t hashed_fingerprint = std::hash<std::string>{}(owner_fingerprint);
    if (owner->get_hashed_fingerprint() != hashed_fingerprint) {
        // 指纹不匹配，抛出异常
        throw std::invalid_argument("Invalid fingerprint for account deletion.");
    }

    // 检查账户是否有未偿还的贷款
    auto unpaidLoanIt = customer_2_unpaid_loan.find(owner);
    if (unpaidLoanIt != customer_2_unpaid_loan.end() && unpaidLoanIt->second > 0) {
        // 有未偿还的贷款，不允许删除账户
        throw std::runtime_error("Cannot delete account with unpaid loan.");
    }

    // 从账户到客户映射中移除账户
    account_2_customer.erase(&account);

    // 从客户到账户映射中移除账户
    customer_2_accounts[owner].erase(std::remove(customer_2_accounts[owner].begin(), customer_2_accounts[owner].end(), &account), customer_2_accounts[owner].end());

    // 从银行账户列表中移除账户
    bank_accounts.erase(std::remove(bank_accounts.begin(), bank_accounts.end(), &account), bank_accounts.end());

    // 删除账户对象
    delete &account;

    // 从银行客户列表中移除客户（如果客户没有其他账户）
    if (customer_2_accounts[owner].empty()) {
        bank_customers.erase(std::remove(bank_customers.begin(), bank_customers.end(), owner), bank_customers.end());
    }

    return true;
}

// 删除客户，验证指纹
bool Bank::delete_customer(Person &owner, const std::string &owner_fingerprint) {
    if (owner.get_hashed_fingerprint() == std::hash<std::string>{}(owner_fingerprint)) {
        // 删除该客户的所有账户
        auto &accounts = customer_2_accounts[&owner];
        for (Account *account : accounts) {
            delete_account(*account, owner_fingerprint);  // 删除账户
        }
        customer_2_accounts.erase(&owner);  // 删除客户的账户记录
        return true;
    }
    
    return false;
}


// 存款操作，需验证指纹
bool Bank::deposit(Account &account, const std::string &owner_fingerprint, double amount) {
    if (account.get_owner()->get_hashed_fingerprint() == std::hash<std::string>{}(owner_fingerprint)) {
        // 只有当金额有效时，才更新账户余额
        if (amount > 0) {
            account.balance += amount;
            // 注意：这里不更新 bank_total_balance，因为它只反映贷款的利息利润
            return true;
        }
    }
    return false;
}
// 取款操作，需验证指纹
bool Bank::withdraw(Account &account, const std::string &owner_fingerprint, double amount) {
    // 验证指纹
    if (account.get_owner()->get_hashed_fingerprint() != std::hash<std::string>{}(owner_fingerprint)) {
        // 指纹不匹配，抛出异常
        throw std::invalid_argument("Invalid fingerprint for withdrawal.");
    }

    // 检查取款金额是否超过账户余额
    if (amount > account.get_balance() || amount <= 0) {
        // 取款金额超过账户余额或不合法，抛出异常
        throw std::invalid_argument("Insufficient funds or invalid withdrawal amount.");
    }

    // 执行取款操作
    account.balance -= amount;
    // 更新银行的总余额
    bank_total_balance -= amount;
    return true;
}

// 转账操作，需验证指纹和账户信息
bool Bank::transfer(Account &source, Account &destination, const std::string &owner_fingerprint, const std::string &CVV2, const std::string &password, const std::string &exp_date, double amount)
{
    std::string non_const_fingerprint = owner_fingerprint;

    if (source.get_owner()->get_hashed_fingerprint() == std::hash<std::string>{}(owner_fingerprint) &&
        source.get_CVV2(non_const_fingerprint) == CVV2 &&
        source.get_password(non_const_fingerprint) == password &&
        source.get_exp_date(non_const_fingerprint) == exp_date &&
        source.get_balance() >= amount)
    {
        // 扣除源账户的金额并增加目标账户的金额
        source.balance -= amount;
        destination.balance += amount;
        return true;
    }
    return false;
}

// 贷款操作，需验证指纹
bool Bank::take_loan(Account &account, const std::string &owner_fingerprint, double amount) {
    if (account.get_owner()->get_hashed_fingerprint() == std::hash<std::string>{}(owner_fingerprint)) {
        double availableLoan = account.get_balance() * 0.6; // 假设最大贷款额度为账户余额的60%
        if (amount > availableLoan) {
            throw std::runtime_error("Insufficient eligibility for loan.");
        }

        double interestRate = 10.0 / account.get_owner()->get_socioeconomic_rank();
        double loanAmountWithInterest = amount * (1 + interestRate / 100.0);

        account.balance += amount;
        Person *owner = const_cast<Person *>(account.get_owner());
        customer_2_unpaid_loan[owner] += loanAmountWithInterest;
        bank_total_loan += loanAmountWithInterest;
        return true;
    }
    return false;
}
// 偿还贷款
bool Bank::pay_loan(Account &account, double amount) {
    // 获取账户所有者，这里假设get_owner()返回的是const Person*，因此使用const_cast来去除const
    Person *owner = const_cast<Person *>(account.get_owner());
    
    // 检查账户余额是否足够还款，以及客户是否有足够未偿还的贷款
    if (customer_2_unpaid_loan[owner] >= amount && account.get_balance() >= amount) {
        // 扣除还款金额
        account.balance -= amount;
        customer_2_unpaid_loan[owner] -= amount;
        customer_2_paid_loan[owner] += amount;
        bank_total_loan -= amount;

        // 检查客户支付的贷款总额是否达到升级其社会经济等级的门槛
        double threshold = std::pow(10, owner->get_socioeconomic_rank());
        if (customer_2_paid_loan[owner] >= threshold) {
            // 如果达到阈值，将客户的等级提高1
            size_t newRank = owner->get_socioeconomic_rank() + 1;
            // 确保等级不会超过10
            if (newRank > 10) {
                newRank = 10;
            }
            owner->set_socioeconomic_rank(newRank);
        }
        return true;
    }
    return false;
}
// 获取银行名称
const std::string &Bank::get_bank_name() const
{
    return bank_name;
}

// 获取银行指纹的哈希值
size_t Bank::get_hashed_bank_fingerprint() const
{
    return hashed_bank_fingerprint;
}

// 获取银行客户列表，需银行验证
const std::vector<Person *> &Bank::get_bank_customers(std::string &bank_fingerprint) const
{
    if (std::hash<std::string>{}(bank_fingerprint) == hashed_bank_fingerprint)
    {
        return bank_customers;
    }
    throw std::runtime_error("银行指纹验证失败");
}

// 获取银行账户列表，需银行验证
const std::vector<Account *> &Bank::get_bank_accounts(std::string &bank_fingerprint) const
{
    if (std::hash<std::string>{}(bank_fingerprint) == hashed_bank_fingerprint)
    {
        return bank_accounts;
    }
    throw std::runtime_error("银行指纹验证失败");
}

// 输出银行信息，支持写入到文件
void Bank::get_info(std::optional<std::string> file_name) const
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

    *out << "银行名称: " << bank_name << std::endl;
    *out << "总资产: " << bank_total_balance << std::endl;
    *out << "总贷款: " << bank_total_loan << std::endl;

    if (file.is_open())
    {
        file.close();
    }
}
// 返回银行的总余额
double Bank::get_bank_total_balance(std::string &bank_fingerprint) const
{
    if (std::hash<std::string>{}(bank_fingerprint) == hashed_bank_fingerprint)
    {
        return bank_total_balance;
    }
    throw std::runtime_error("银行指纹验证失败");
}

// 返回银行的总贷款金额
double Bank::get_bank_total_loan(std::string &bank_fingerprint) const
{
    if (std::hash<std::string>{}(bank_fingerprint) == hashed_bank_fingerprint)
    {
        return bank_total_loan;
    }
    throw std::runtime_error("银行指纹验证失败");
}

// 获取账户到客户映射
const std::map<Account *, Person *> &Bank::get_account_2_customer_map(std::string &bank_fingerprint) const
{
    if (std::hash<std::string>{}(bank_fingerprint) == hashed_bank_fingerprint)
    {
        return account_2_customer;
    }
    throw std::runtime_error("银行指纹验证失败");
}

// 获取客户到账户映射
const std::map<Person *, std::vector<Account *>> &Bank::get_customer_2_accounts_map(std::string &bank_fingerprint) const
{
    if (std::hash<std::string>{}(bank_fingerprint) == hashed_bank_fingerprint)
    {
        return customer_2_accounts;
    }
    throw std::runtime_error("银行指纹验证失败");
}

// 获取客户未还贷款映射
const std::map<Person *, double> &Bank::get_customer_2_unpaid_loan_map(std::string &bank_fingerprint) const
{
    if (std::hash<std::string>{}(bank_fingerprint) == hashed_bank_fingerprint)
    {
        return customer_2_unpaid_loan;
    }
    throw std::runtime_error("银行指纹验证失败");
}

// 获取客户已还贷款映射
const std::map<Person *, double> &Bank::get_customer_2_paid_loan_map(std::string &bank_fingerprint) const
{
    if (std::hash<std::string>{}(bank_fingerprint) == hashed_bank_fingerprint)
    {
        return customer_2_paid_loan;
    }
    throw std::runtime_error("银行指纹验证失败");
}