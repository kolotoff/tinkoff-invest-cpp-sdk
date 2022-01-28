#ifndef CREDENTIALS_HPP
#define CREDENTIALS_HPP

#include <string>
#include <fstream>

class Credentials
{
  enum { MinimalFileSize = 8 };

public:
  Credentials() = default;
  ~Credentials() = default;

  const std::string& certs() const { return certs_; }
  const std::string& token() const { return token_; }
  const std::string& account() const { return account_; }

  bool load()
  {
    const std::string rootCertFilePath = "roots.pem";
    const std::string tokenFilePath = "token.txt";
    const std::string accountFilePath = "account.txt";

    if (!readFile(rootCertFilePath, certs_))
    {
      return false;
    }

    if (!readFile(tokenFilePath, token_))
    {
      return false;
    }

    if (!readFile(accountFilePath, account_))
    {
      return false;
    }

    return true;
  }

private:
  bool readFile(const std::string& filePath, std::string& buffer)
  {
    std::ifstream srcFile(filePath, std::ios::binary);
    if (!srcFile.is_open())
    {
      return false;
    }

    std::string tmp((std::istreambuf_iterator<char>(srcFile)), std::istreambuf_iterator<char>());
    if (tmp.size() < MinimalFileSize)
    {
      return false;
    }

    buffer = tmp;

    return true;
  }

private:
  std::string certs_;
  std::string token_;
  std::string account_;
};


#endif // !CREDENTIALS_HPP