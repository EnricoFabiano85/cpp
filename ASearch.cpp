#include<array>
#include<algorithm>
#include<cassert>
#include<concepts>
#include <functional>
#include<iostream>
#include<limits>
#include<vector>
#include<queue>

// https://coursera.cs.princeton.edu/algs4/assignments/8puzzle/specification.php

template<int N>
class Board
{
private: 
  using ABoard = std::array<int, N*N>;

public:

  template<std::same_as<int>... T>
  Board(T... args) noexcept : _board{args...}
  {}

  int hamming() const
  {
    int hamming = 0;
    for (int index = 0; index < N*N-1; ++index)
      if (_board[index] != index+1) ++hamming;

    return hamming;
  }

  int manhattan() const
  {
    int value = 0;
    for (int index = 0; index < N*N; ++index)
    {
      int const tile = _board[index];
      if (tile != 0)
      {
        int const distance = std::abs(index+1 - tile);
        value += distance/N + distance%N;
      }
    }

    return value;      
  }

  bool isGoal() const
  { return hamming()==0 ? true : false;}

  auto neighbors() const
  {
    std::array<Board, 4> neighbors;
    neighbors.fill(*this);

    auto const zero = findZero();
    int const row = zero/N;
    int const col = zero%N;
    int constexpr size = N-1;

    if (col < size) std::swap(neighbors[0]._board[zero], neighbors[0]._board[zero+1]);
    if (col > 0) std::swap(neighbors[1]._board[zero], neighbors[1]._board[zero-1]);
    if (row < size) std::swap(neighbors[2]._board[zero], neighbors[2]._board[zero+N]);
    if (row > 0) std::swap(neighbors[3]._board[zero], neighbors[3]._board[zero-N]);

    return neighbors;
  }

  bool isSolvable() const
  { 
    // https://math.stackexchange.com/posts/2842654/revisions
    return (inversions()+distanceFromLowerRight())%2 == 0 ? true : false;
  }

  friend std::ostream& operator<<(std::ostream& stream, const Board &board)
  {
    stream << N;
    for(int row = 0; row < N; ++row)
    {
      stream << std::endl;
      for(int col = 0; col < N; ++col)
        stream << board._board[row*N+col] << " ";
    }
    stream << std::endl;
    return stream;
  }

  friend bool operator==(Board const &lhs, Board const &rhs)
  { return lhs._board == rhs._board; }

  friend bool operator!=(Board const &lhs, Board const &rhs)
  { return !(lhs._board == rhs._board); }

  friend bool operator<(Board const &lhs, Board const &rhs)
  { return lhs.manhattan() < rhs.manhattan(); }

  friend bool operator>(Board const &lhs, Board const &rhs)
  { return lhs.manhattan() > rhs.manhattan(); }

private:
  ABoard _board;

  int inversions() const
  {
    int constexpr maxInt = std::numeric_limits<int>::max();
    int inversions = 0;
    for (int index1 = 0; index1 < (N*N)-1; ++index1)
    {
      int const first = (_board[index1] == 0) ? maxInt : _board[index1];
      for (int index2 = index1+1; index2 < N*N; ++index2)
      {
        int const second = (_board[index2] == 0) ? maxInt : _board[index2];
        if (first > second) ++inversions;
      }
    }

    return inversions;
  }

  int distanceFromLowerRight() const
  {
    int const distanceFromLowerRight = findZero() - N*N -1;
    return distanceFromLowerRight/N+distanceFromLowerRight%N;
  }

  int constexpr findZero() const
  { return std::distance(_board.cbegin(), std::find(_board.cbegin(), _board.cend(), 0)); };
};


template<int N>
class ASearch
{
  using ABoard = Board<N>;
public:
  ASearch(ABoard const &initial) noexcept : _initialBoard(std::move(initial))
  {}

  void solve()
  {
    if (_initialBoard.isSolvable())
    {
      std::priority_queue<ABoard, std::vector<ABoard>, std::greater<ABoard>> pq;
      pq.push(_initialBoard);
      auto &previousTop = _initialBoard;

      while(!pq.empty())
      {
        _solutions.emplace_back(pq.top());
        auto const &currentTop = _solutions.back();
        ++_moves;
        if (currentTop.isGoal()) break;

        pq.pop();

        for (auto const &aBoard : currentTop.neighbors())
        {
          if ((aBoard != currentTop) && (aBoard != previousTop)) pq.push(aBoard);
        }
        previousTop = currentTop;
      }
    }
  }

  int moves() const
  { return _moves; }

  void print()
  { std::cout << _initialBoard << std::endl; }

private:
  ABoard _initialBoard;
  int _moves{-1}; 
  std::vector<ABoard> _solutions{};
};

int main()
{
  int constexpr rows = 3;
  auto board = Board<rows>{0,1,3,4,2,5,7,8,6};

  ASearch<rows> solver(board);
  solver.solve();

  assert(solver.moves() == 4);
}