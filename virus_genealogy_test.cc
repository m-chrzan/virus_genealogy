#include <algorithm>
#include <string>
#include <vector>
#include "testing.h"
#include "virus_genealogy.h"
#include "sample_virus.h"

VirusGenealogy<Virus<std::string>> oneVirusGenealogy() {
    return VirusGenealogy<Virus<std::string>>("A");
}


VirusGenealogy<Virus<std::string>> smallGenealogy() {
    VirusGenealogy<Virus<std::string>> vg("A");
    vg.create("B", "A");
    vg.create("C", "A");
    vg.create("D", "A");
    vg.create("AB", std::vector<std::string>{"A", "B"});
    vg.create("CD", std::vector<std::string>{"C", "D"});
    vg.create("ABCD", std::vector<std::string>{"AB", "CD"});
    vg.create("E", "B");
    vg.create("F", "CD");

    return vg;
}

void testGetStemId() {
    beginTest();

    VirusGenealogy<Virus<std::string>> vg1 = oneVirusGenealogy();
    checkEqual<std::string>(vg1.get_stem_id(), "A",
            "Stem id remembered correctly.");

    VirusGenealogy<Virus<std::string>> vg2 = smallGenealogy();
    checkEqual<std::string>(vg2.get_stem_id(), "A",
            "Stem id remembered correctly.");
}

int main() {
    testGetStemId();
}
