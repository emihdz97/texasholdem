#include <iostream>
#include <stdlib.h>
#include "SevenEval.h"

int main(int argc, char* argv[]) {

	if (argc != 8) return -1;

	int card1 = atof(argv[1]);
    int card2 = atof(argv[2]);
    int card3 = atof(argv[3]);
    int card4 = atof(argv[4]);
    int card5 = atof(argv[5]);
    int card6 = atof(argv[6]);
    int card7 = atof(argv[7]);


  // Get the rank of the seven-card spade flush, ace high.
  std::cout << SevenEval::GetRank(card1, card2, card3, card4, card5, card6, card7) << std::endl;

  return 0;
}