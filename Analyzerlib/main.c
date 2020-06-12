#include "analyzer.h"

enum cards{
    AceSpades,
    AceHearts,
    AceDimonds,
    AceClubs,

    KingSpades,
    KingHearts,
    KingDiamonds,
    KingClubs,

    QueenSpades,
    QueenHearts,
    QueenDiamonds,
    QueenClubs,

    JackSpades,
    JackHearts,
    JackDiamonds,
    JackClubs,

    TenSpades,
    TenHearts,
    TenDiamonds,
    TenClubs,

    NineSpades,
    NineHearts,
    NineDiamonds,
    NineClubs,

    EightSpades,
    EightHearts,
    EightDiamonds,
    EightClubs,

    SevenSpades,
    SevenHearts,
    SevenDiamonds,
    SevenClubs,

    SixSpades,
    SixHearts,
    SixDiamonds,
    SixClubs,

    FiveSpades,
    FiveHearts,
    FiveDiamonds,
    FiveClubs,

    FourSpades,
    FourHearts,
    FourDiamonds,
    FourClubs,

    ThreeSpades,
    ThreeHearts,
    ThreeDiamonds,
    ThreeClubs,

    TwoSpades,
    TwoHearts,
    TwoDiamonds,
    TwoClubs

};

int main () {



enum cards current_card=AceSpades;

	printf("Value of TenSpades= %d\n",current_card );




	//Usage
    getScore(0, 4, 8, 12, 16, 20, 24);



   return(0);
} 


