#include <iostream>
using namespace std;

#include "solution.h"
int binarySearch(int *input, int n, int val) {
  int s = 0;
  int e = n;
  while (s <= e) {
    int mid = (s + e) / 2;
    if (input[mid] == val) {
      return mid;
    } else if (input[mid] > val) {
      e = mid - 1;
    } else {
      s = mid + 1;
    }
  }
  return -1;
}

int main() {
  int size;
  cin >> size;
  int *input = new int[size];

  for (int i = 0; i < size; ++i) {
    cin >> input[i];
  }

  int t;
  cin >> t;

  while (t--) {
    int val;
    cin >> val;
    cout << binarySearch(input, size, val) << endl;
  }

  delete[] input;
  return 0;
}