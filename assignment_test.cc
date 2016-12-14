#include "virus_genealogy.h"
#include "sample_virus.h"

int main() {
    VirusGenealogy<Virus<std::string>> vg1("A");

    VirusGenealogy<Virus<std::string>> vg2("B");

    vg2 = vg1;
}
