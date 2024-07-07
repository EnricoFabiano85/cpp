#include <iostream>
#include <variant>
#include <vector>

struct Pet 
{
  template<typename Visitor>
  void accept(this auto &&self, Visitor &v)
  { v.visit(self); }
};

struct Cat : Pet {};
struct Dog : Pet {};
struct Wolf : Pet {};

struct FeedingVisitor
{
  void visit(Cat const &) const
  { std::cout << "Feeding cat -- template visitor " << std::endl; }

  void visit(Dog const &) const 
  { std::cout << "Feeding dog -- template visitor " << std::endl; }

  void visit(Wolf const &) const
  { std::cout << "Feeding wolf -- template visitor " << std::endl; }
};

struct FeedingPets {
  void operator()(Cat &) { std::cout << "Feeding a cat" << std::endl; }
  void operator()(Dog &) { std::cout << "Feeding a dog" << std::endl; }
  void operator()(Wolf &) { std::cout << "Feeding a wolf" << std::endl; }
};

struct PettingPets {
  void operator()(Cat &) { std::cout << "Petting a cat" << std::endl; }
  void operator()(Dog &) { std::cout << "Petting a dog" << std::endl; }
  void operator()(Wolf &) { std::cout << "Petting a wolf" << std::endl; }
};

int main() {
  auto vec = std::vector<std::variant<Cat, Dog, Wolf>>{Cat{}, Wolf{}, Dog{}};
  for (auto &e : vec)
  {
    std::visit(FeedingPets(), e);
    std::visit(PettingPets(), e);
  }

  std::cout << "---------------- COMPILE TIME VISITOR ----------------" << std::endl;
  FeedingVisitor fv;
  auto const cat = Cat();
  cat.accept(fv);

  auto const &dog = Dog();
  dog.accept(fv);
}