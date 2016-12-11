#ifndef SAMPLE_VIRUS_H
#define SAMPLE_VIRUS_H

#include <string>

template<typename T>
class Virus {
  public:
    typedef T id_type;
    Virus(id_type const &_id) : id(_id) {
    }
    id_type get_id() const {
      return id;
    }
  private:
    id_type id;
};

#endif
