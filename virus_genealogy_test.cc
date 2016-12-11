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

void testGetChildren() {
    beginTest();

    VirusGenealogy<Virus<std::string>> vg1 = oneVirusGenealogy();

    std::vector<std::string> children = vg1.get_children("A");
    std::vector<std::string> expected_children;
    checkEqual(children, expected_children, "Stem has no children.");

    checkExceptionThrown<VirusNotFound>([&vg1] { vg1.get_children("B"); },
            "Can't get children of virus not in the genealogy.");

    VirusGenealogy<Virus<std::string>> vg2 = smallGenealogy();

    children = vg2.get_children("A");
    expected_children = {"AB", "B", "C", "D"};
    checkSameSet(children, expected_children,
            "Got the correct children for the stem.");

    children = vg2.get_children("CD");
    expected_children = {"ABCD", "F"};
    checkSameSet(children, expected_children,
            "Got the correct children for an inner node.");

    children = vg2.get_children("E");
    expected_children = {};
    checkSameSet(children, expected_children, "Leaf has no children.");

    checkExceptionThrown<VirusNotFound>([&vg2] { vg2.get_children("AC"); },
            "Can't get children of virus not in the genealogy.");
}

void testGetParents() {
    beginTest();

    VirusGenealogy<Virus<std::string>> vg1 = oneVirusGenealogy();

    std::vector<std::string> parents = vg1.get_parents("A");
    std::vector<std::string> expected_parents;
    checkSameSet(parents, expected_parents, "Stem has no parents.");

    checkExceptionThrown<VirusNotFound>([&vg1] { vg1.get_parents("B"); },
            "Can't get parents of virus not in the genealogy.");

    VirusGenealogy<Virus<std::string>> vg2 = smallGenealogy();

    parents = vg2.get_parents("A");
    expected_parents = {};
    checkSameSet(parents, expected_parents, "Stem has no parents.");

    parents = vg2.get_parents("B");
    expected_parents = {"A"};
    checkSameSet(parents, expected_parents, "Got the correct parent.");

    parents = vg2.get_parents("ABCD");
    expected_parents = {"AB", "CD"};
    checkSameSet(parents, expected_parents, "Got the correct parents.");

    checkExceptionThrown<VirusNotFound>([&vg2] { vg2.get_parents("42"); },
            "Can't get parents of virus not in the genealogy.");
}

int main() {
    testGetStemId();
    testGetChildren();
    testGetParents();
}
