#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

class VirusNotFound : public std::exception {
    const char* what() const noexcept {
        return "Virus not found!";
    }
};

class VirusAlreadyCreated : public std::exception {
    const char* what() const noexcept {
        return "Virus already created!";
    }
};

class TriedToRemoveStemVirus : public std::exception {
    const char* what() const noexcept {
        return "Can't remove stem virus!";
    }
};

template<class Virus>
class VirusGenealogy{
private:
    class Node;

    typename Virus::id_type stem_id_;
    std::map<typename Virus::id_type, std::shared_ptr<Node>> genealogy_;

    void throwIfNotFound(typename Virus::id_type const &id) const {
        if (!exists(id)) {
            throw VirusNotFound();
        }
    }

    void throwIfAlreadyCreated(typename Virus::id_type const &id) const {
        if (exists(id)) {
            throw VirusAlreadyCreated();
        }
    }

    class Node {
		typedef typename std::map<typename Virus::id_type, std::shared_ptr<Node>>::iterator map_iter;
    public:
        Node(typename Virus::id_type const &id) : virus_(id), id_(id) {
        }

        void add_child(std::shared_ptr<Node> &child) {
            std::weak_ptr<Node> child_wp(child); // const
            children_.insert(child_wp); // strong
        } // therefore strong

        void add_parent(std::shared_ptr<Node> &parent) {
            std::weak_ptr<Node> parent_wp(parent); //const
            parents_.insert(parent_wp); // strong
        } // therefore strong

        void remove_child(std::shared_ptr<Node> &child) {
            std::weak_ptr<Node> child_wp(child); // no-throw
            children_.erase(child_wp); // no-throw
        } // therefore no-throw

        void remove_parent(std::shared_ptr<Node> &parent) {
            std::weak_ptr<Node> parent_wp(parent); // no-throw
			parents_.erase(parent_wp); // no-throw
        } // therefore no-throw
        
        bool edge_exists(std::shared_ptr<Node> &parent) {
			return parents_.find(parent) != parents_.end(); // strong
		}

        std::vector<typename Virus::id_type> get_children() const {
            std::vector<typename Virus::id_type> children_ids;

            for (std::weak_ptr<Node> node_wp : children_) {
                std::shared_ptr<Node> node_sp = node_wp.lock();
                children_ids.push_back(node_sp->id_);
            }

            return children_ids;
        }

        std::vector<typename Virus::id_type> get_parents() const {
            std::vector<typename Virus::id_type> parent_ids;

            for (std::weak_ptr<Node> node_wp : parents_) {
                std::shared_ptr<Node> node_sp = node_wp.lock();
                parent_ids.push_back(node_sp->id_);
            }

            return parent_ids;
        }

        Virus& get_virus() {
            return virus_;
        }

        typename Virus::id_type get_id() {
            return id_;
        }
        
        map_iter get_iter() {
			return iter;
		}
			
		void set_iter(map_iter it) {
			iter = it;
		}
		
    private:
        Virus virus_;
        typename Virus::id_type id_;
        map_iter iter;

        std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> children_;
        std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> parents_;
    };
public:
    VirusGenealogy(typename Virus::id_type const &stem_id) : stem_id_(stem_id) {
        std::shared_ptr<Node> node_sp = std::make_shared<Node>(Node(stem_id));
        genealogy_.insert(std::make_pair(stem_id, node_sp));
    };

    typename Virus::id_type get_stem_id() const {
        return stem_id_;
    }

    std::vector<typename Virus::id_type>
        get_children(typename Virus::id_type const &id) const {
        throwIfNotFound(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second; // strong
        return node_sp->get_children();
    }

    std::vector<typename Virus::id_type> get_parents(typename Virus::id_type const &id) const {
        throwIfNotFound(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second; // strong
        return node_sp->get_parents();
    }

    bool exists(typename Virus::id_type const &id) const {
        return genealogy_.find(id) != genealogy_.end(); // strong
    }

    Virus& operator[](typename Virus::id_type const &id) const {
        throwIfNotFound(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second; // strong
        return node_sp->get_virus();
    }

    void create(typename Virus::id_type const &id,
            typename Virus::id_type const &parent_id) {
        throwIfAlreadyCreated(id);
        throwIfNotFound(parent_id);

        std::shared_ptr<Node> node_sp = std::make_shared<Node>(id); //const
        auto it = genealogy_.insert(std::make_pair(id, node_sp)).first; // strong

        try {
			node_sp->set_iter(it);
            connect(id, parent_id); // strong
        } catch (...) {
			node_sp->set_iter(genealogy_.end()); // temporary
            genealogy_.erase(it); // no-throw
        }
    } // try-catch-reverse makes the whole function strong

    void create(typename Virus::id_type const &id,
            std::vector<typename Virus::id_type> const &parent_ids) {
        throwIfAlreadyCreated(id);
        
        std::vector<std::shared_ptr<Node>> nodes;
        for (size_t i = 0; i < parent_ids.size(); i++) {
            throwIfNotFound(parent_ids[i]);
        }
        
         for (size_t i = 0; i < parent_ids.size(); i++) {
            nodes.push_back(genealogy_.find(parent_ids[i])->second); 
        }
        
		// if push_back throws then it's okay
        std::shared_ptr<Node> node_sp = std::make_shared<Node>(id); // const
        auto it = genealogy_.insert(std::make_pair(id, node_sp)).first; // strong
		
		try {
			node_sp->set_iter(it);
			for (size_t i = 0; i < parent_ids.size(); i++) {
				connect(id, parent_ids[i]);
			}
		} catch (...) {
			node_sp->set_iter(genealogy_.end());
			genealogy_.erase(it);
			for (size_t i = 0; i < nodes.size(); i++) {
				nodes[i]->remove_child(node_sp);
				node_sp->remove_parent(nodes[i]);
			}
		}
    }

    void connect(typename Virus::id_type const &child_id,
            typename Virus::id_type const &parent_id) {
        throwIfNotFound(child_id);
        throwIfNotFound(parent_id);

        std::shared_ptr<Node> child_sp = genealogy_.find(child_id)->second; // const

        std::shared_ptr<Node> parent_sp = genealogy_.find(parent_id)->second; // const
        
        // strong, but if throws we don't care
        if (!child_sp->edge_exists(parent_sp)) {
			child_sp->add_parent(parent_sp); // strong
			try {
				parent_sp->add_child(child_sp); // strong
			} catch (...) {
				child_sp->remove_parent(parent_sp); // no-throw
				throw;
			}
		}
    } // try-catch-reverse makes whole function strong

    void remove(typename Virus::id_type const &id) {
        if (id == stem_id_) {
           throw TriedToRemoveStemVirus();
        }

        throwIfNotFound(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second; // const

        std::vector<typename Virus::id_type> parent_ids = get_parents(id); // const?
        for (typename Virus::id_type parent_id : parent_ids) {
            std::shared_ptr<Node> parent_sp = genealogy_.find(parent_id)->second;
            parent_sp->remove_child(node_sp);
        }

        std::vector<typename Virus::id_type> children_ids = get_children(id);
        for (typename Virus::id_type child_id : children_ids) {
            std::shared_ptr<Node> child_sp = genealogy_.find(child_id)->second;
            child_sp->remove_parent(node_sp);

            if (child_sp->get_parents().size() == 0) {
                remove(child_sp->get_id());
            }
        }

        genealogy_.erase(id);
    }
};

#endif
