#pragma once

namespace wedge {

class Account {
 public:
  explicit Account(double balance, double position)
      : balance_(balance), position_(position) {}

  double balance() const { return balance_; }
  double position() const { return position_; }

  void update_balance(double value) { balance_ += value; }
  void update_position(double value) { position_ += value; }

 private:
  double balance_;
  double position_;
};

}  // namespace wedge