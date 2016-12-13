#include <algorithm>
#include <string>
#include <vector>
#include "testing.h"
#include "virus_genealogy.h"
#include "sample_virus.h"

class SingleVirusGenealogy : public VirusGenealogy<Virus<std::string>> {
public:
    SingleVirusGenealogy() : VirusGenealogy("A") {}
};

class SmallGenealogy : public VirusGenealogy<Virus<std::string>> {
public:
    SmallGenealogy() : VirusGenealogy("A") {
        create("B", "A");
        create("C", "A");
        create("D", "A");
        create("AB", std::vector<std::string>{"A", "B"});
        create("CD", std::vector<std::string>{"C", "D"});
        create("ABCD", std::vector<std::string>{"AB", "CD"});
        create("E", "B");
        create("F", "CD");
    }
};

template<typename Virus>
void checkAllExist(VirusGenealogy<Virus> &vg,
        std::vector<typename Virus::id_type> &ids,
        std::string message) {
    bool allExist = true;

    std::for_each(ids.begin(), ids.end(), [&vg, &allExist](typename Virus::id_type id) {
            allExist = (allExist && vg.exists(id));
            });

    check(allExist, message);
}

void testGetStemId() {
    beginTest();

    SingleVirusGenealogy singleVirus;
    checkEqual<std::string>(singleVirus.get_stem_id(), "A",
            "Stem id remembered correctly.");

    SmallGenealogy smallGenealogy;
    checkEqual<std::string>(smallGenealogy.get_stem_id(), "A",
            "Stem id remembered correctly.");
}

void testGetChildren() {
    beginTest();

    SingleVirusGenealogy singleVirus;

    std::vector<std::string> children = singleVirus.get_children("A");
    std::vector<std::string> expected_children;
    checkEqual(children, expected_children, "Stem has no children.");

    checkExceptionThrown<VirusNotFound>([&singleVirus] { singleVirus.get_children("B"); },
            "Can't get children of virus not in the genealogy.");

    SmallGenealogy smallGenealogy;

    children = smallGenealogy.get_children("A");
    expected_children = {"AB", "B", "C", "D"};
    checkSameSet(children, expected_children,
            "Got the correct children for the stem.");

    children = smallGenealogy.get_children("CD");
    expected_children = {"ABCD", "F"};
    checkSameSet(children, expected_children,
            "Got the correct children for an inner node.");

    children = smallGenealogy.get_children("E");
    expected_children = {};
    checkSameSet(children, expected_children, "Leaf has no children.");

    checkExceptionThrown<VirusNotFound>([&smallGenealogy] { smallGenealogy.get_children("AC"); },
            "Can't get children of virus not in the genealogy.");
}

void testGetParents() {
    beginTest();

    SingleVirusGenealogy singleVirus;

    std::vector<std::string> parents = singleVirus.get_parents("A");
    std::vector<std::string> expected_parents;
    checkSameSet(parents, expected_parents, "Stem has no parents.");

    checkExceptionThrown<VirusNotFound>([&singleVirus] { singleVirus.get_parents("B"); },
            "Can't get parents of virus not in the genealogy.");

    SmallGenealogy smallGenealogy;

    parents = smallGenealogy.get_parents("A");
    expected_parents = {};
    checkSameSet(parents, expected_parents, "Stem has no parents.");

    parents = smallGenealogy.get_parents("B");
    expected_parents = {"A"};
    checkSameSet(parents, expected_parents, "Got the correct parent.");

    parents = smallGenealogy.get_parents("ABCD");
    expected_parents = {"AB", "CD"};
    checkSameSet(parents, expected_parents, "Got the correct parents.");

    checkExceptionThrown<VirusNotFound>([&smallGenealogy] { smallGenealogy.get_parents("42"); },
            "Can't get parents of virus not in the genealogy.");
}

void testExists() {
    beginTest();

    SingleVirusGenealogy singleVirus;

    check(singleVirus.exists("A"), "The original virus exists in the genealogy.");
    checkFalse(singleVirus.exists("B"),
            "A different virus does not exist in the genealogy.");

    SmallGenealogy smallGenealogy;
    std::vector<std::string> expected = {"A", "B", "C", "D", "E", "AB", "CD", "ABCD", "F"};

    checkAllExist(smallGenealogy, expected,
            "Found all genealogy nodes.");
    checkFalse(smallGenealogy.exists("G"), "Virus not in the genealogy doesn't exist.");
}

void testSubscript() {
    beginTest();

    SingleVirusGenealogy singleVirus;

    std::string id = singleVirus["A"].get_id();
    std::string expected_id = "A";
    checkEqual(id, expected_id, "Got the correct virus.");

    checkExceptionThrown<VirusNotFound>([&singleVirus]{ singleVirus["B"]; },
            "Can't index with a virus id not in the genealogy.");

    SmallGenealogy smallGenealogy;

    id = smallGenealogy["A"].get_id();
    expected_id = "A";
    checkEqual(id, expected_id, "Got the correct virus.");

    id = smallGenealogy["B"].get_id();
    expected_id = "B";
    checkEqual(id, expected_id, "Got the correct virus.");

    id = smallGenealogy["ABCD"].get_id();
    expected_id = "ABCD";
    checkEqual(id, expected_id, "Got the correct virus.");
}

void testCreate() {
    beginTest();

    SingleVirusGenealogy singleVirus;

    checkExceptionThrown<VirusAlreadyCreated>([&singleVirus] { singleVirus.create("A", "A"); },
            "Can't create virus that already exists.");

    checkExceptionThrown<VirusNotFound>([&singleVirus] { singleVirus.create("C", "B"); },
            "Virus can't descend from virus that doesn't exist.");

    singleVirus.create("B", "A");

    check(singleVirus.exists("B"), "New virus exists.");

    std::vector<std::string> children = singleVirus.get_children("A");
    std::vector<std::string> expected_children = {"B"};
    checkSameSet(children, expected_children, "Stem has a new child.");

    std::vector<std::string> parents = singleVirus.get_parents("B");
    std::vector<std::string> expected_parents = {"A"};
    checkSameSet(parents, expected_parents, "New virus's parent set correctly.");

    singleVirus.create("AB", std::vector<std::string> {"A", "B"});

    children = singleVirus.get_children("A");
    expected_children = {"B", "AB"};
    checkSameSet(children, expected_children, "Stem has a new child.");

    children = singleVirus.get_children("B");
    expected_children = {"AB"};
    checkSameSet(children, expected_children, "New virus a new child.");

    parents = singleVirus.get_parents("AB");
    expected_parents = {"A", "B"};
    checkSameSet(parents, expected_parents, "New virus's parents set correctly.");
}

void testRemove() {
    beginTest();

    SingleVirusGenealogy singleVirus;

    checkExceptionThrown<TriedToRemoveStemVirus>(
            [&singleVirus] { singleVirus.remove("A"); },
            "Can't remove stem.");

    checkExceptionThrown<VirusNotFound>(
            [&singleVirus] { singleVirus.remove("B"); },
            "Can't remove a virus that's not in the genealogy.");

    SmallGenealogy smallGenealogy;

    checkExceptionThrown<TriedToRemoveStemVirus>(
            [&singleVirus] { singleVirus.remove("A"); },
            "Can't remove stem.");

    checkExceptionThrown<VirusNotFound>(
            [&singleVirus] { singleVirus.remove("ABC"); },
            "Can't remove a virus that's not in the genealogy.");

    smallGenealogy.remove("E");
    checkFalse(smallGenealogy.exists("E"), "A leaf virus was removed.");

    std::vector<std::string> children = smallGenealogy.get_children("B");
    std::vector<std::string> expected_children = {"AB"};

    checkSameSet(children, expected_children,
            "The removed virus's parent no longer has it as a child.");

    smallGenealogy.remove("CD");
    checkFalse(smallGenealogy.exists("CD"), "Removed inner virus.");
    checkFalse(smallGenealogy.exists("F"), "Removed orphaned virus.");
    check(smallGenealogy.exists("ABCD"), "Virus with second parent still exists.");

    std::vector<std::string> parents = smallGenealogy.get_parents("ABCD");
    std::vector<std::string> expected_parents = {"AB"};

    checkSameSet(parents, expected_parents, "Virus has only one parent left.");
}

int main() {
    testGetStemId();
    testExists();
    testGetParents();
    testGetChildren();
    testCreate();
    testSubscript();
    testRemove();
}
