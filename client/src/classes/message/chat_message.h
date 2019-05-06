//
// Created by Augens on 06/05/2019.
//

#ifndef CLIENT_CHAT_MESSAGE_H
#define CLIENT_CHAT_MESSAGE_H



#include <cstdio>
#include <cstdlib>
#include <cstring>

class chat_message
{
public:
  enum { header_length = 4 };
  enum { max_body_length = 512 };

  chat_message();

  const char* data() const;
  char* data();

  size_t length() const;

  const char* body() const;

  char* body();

  size_t body_length() const;

  void body_length(size_t length);

  bool decode_header();

  void encode_header();

private:
  char data_[header_length + max_body_length];
  size_t body_length_;
};

#endif //CLIENT_CHAT_MESSAGE_H
