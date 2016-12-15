#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <iostream>
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

    void throw_if_not_found(typename Virus::id_type const &id) const {
        if (!exists(id)) {
            throw VirusNotFound();
        }
    }

    void throw_if_already_created(typename Virus::id_type const &id) const {
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
			int x = parents_.erase(parent_wp); // no-throw
			std::cout<<x<<"\n";
        } // therefore no-throw
        
        bool edge_exists(std::shared_ptr<Node> &parent) {
			return parents_.find(parent) != parents_.end(); // strong
		}

        std::vector<typename Virus::id_type> get_children() const {
            std::vector<typename Virus::id_type> children_ids(0);

            for (std::weak_ptr<Node> node_wp : children_) {
                std::shared_ptr<Node> node_sp = node_wp.lock();
                children_ids.push_back(node_sp->id_);
            }

            return children_ids;
        }

        std::vector<typename Virus::id_type> get_parents() const {
            std::vector<typename Virus::id_type> parent_ids(0);
			//std::cout<<"here i am " << id_ << "\n";
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
			return iter_;
		}
			
		void set_iter(map_iter it) {
			iter_ = it;
		}
		
    private:
        Virus virus_;
        typename Virus::id_type id_;
        map_iter iter_;

        std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> children_;
        std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> parents_;
    };
public:
    VirusGenealogy(typename Virus::id_type const &stem_id) : stem_id_(stem_id) {
        std::shared_ptr<Node> node_sp = std::make_shared<Node>(Node(stem_id));
        genealogy_.insert(std::make_pair(stem_id, node_sp));
    };
	
	VirusGenealogy(VirusGenealogy &) = delete;

	VirusGenealogy& operator=(const VirusGenealogy &other) = delete;
	
    typename Virus::id_type get_stem_id() const {
        return stem_id_;
    }

    std::vector<typename Virus::id_type>
        get_children(typename Virus::id_type const &id) const {
        throw_if_not_found(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second; // strong
        return node_sp->get_children();
    }

    std::vector<typename Virus::id_type> get_parents(typename Virus::id_type const &id) const {
        throw_if_not_found(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second; // strong
        return node_sp->get_parents();
    }

    bool exists(typename Virus::id_type const &id) const {
        return genealogy_.find(id) != genealogy_.end(); // strong
    }

    Virus& operator[](typename Virus::id_type const &id) const {
        throw_if_not_found(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second; // strong
        return node_sp->get_virus();
    }

    void create(typename Virus::id_type const &id,
            typename Virus::id_type const &parent_id) {
        throw_if_already_created(id);
        throw_if_not_found(parent_id);

        std::shared_ptr<Node> node_sp = std::make_shared<Node>(id); //const
        auto it = genealogy_.insert(std::make_pair(id, node_sp)).first; // strong

        try {
			node_sp->set_iter(it);
            connect(id, parent_id); // strong
        } catch (...) {
            genealogy_.erase(it); // no-throw
        }
    } // try-catch-reverse makes the whole function strong

    void create(typename Virus::id_type const &id,
            std::vector<typename Virus::id_type> const &parent_ids) {
        throw_if_already_created(id);
        
        std::vector<std::shared_ptr<Node>> nodes;
        for (size_t i = 0; i < parent_ids.size(); i++) {
            throw_if_not_found(parent_ids[i]);
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
			genealogy_.erase(it);
			for (size_t i = 0; i < nodes.size(); i++) {
				nodes[i]->remove_child(node_sp);
				node_sp->remove_parent(nodes[i]);
			}
			throw;
		}
    }

    void connect(typename Virus::id_type const &child_id,
            typename Virus::id_type const &parent_id) {
        throw_if_not_found(child_id);
        throw_if_not_found(parent_id);

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
	
	void remove_helper(Node node, std::vector<std::pair<std::shared_ptr<Node>, bool>> &nodes) {
		
		// copy	
		auto id = node.get_id();
		auto node_sp = genealogy_.find(id)->second;
		std::cout<<id<<" - id\n";
		std::vector<typename Virus::id_type> parent_ids = get_parents(id); // const?
		std::cout<<"pp\n";
        for (auto p : parent_ids) {
			std::cout<<p<<" - par\n";
            std::shared_ptr<Node> parent_sp = genealogy_.find(p)->second;
            Node cp = *parent_sp;
            cp.remove_child(node_sp);
            //for (size_t i = 0; i < cp.get_children().size(); i++)
			//	std::cout<<cp.get_children()[i]<<" ";
			//std::cout<<"\n\n";
            nodes.push_back(std::make_pair(std::make_shared<Node>(cp), false));
        }

        std::vector<typename Virus::id_type> children_ids = get_children(id);
        for (size_t i = 0; i < children_ids.size(); i++) {
            std::shared_ptr<Node> child_sp = genealogy_.find(children_ids[i])->second;
            Node cp = *child_sp;
            cp.remove_parent(node_sp);
            std::cout<<"tutaj\n";
            nodes.push_back(std::make_pair(std::make_shared<Node>(cp), false));

            if (cp.get_parents().size() == 0) {
                remove(child_sp->get_id());
            }
        }
			
		nodes.push_back(std::make_pair(node_sp, true));
		std::cout<<"dodtad\n";
	}
		
    void remove(typename Virus::id_type const &id) {
        if (id == stem_id_) {
           throw TriedToRemoveStemVirus();
        }
		std::cout<<id<<"\n";
        throw_if_not_found(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second; // const
		Node node_to_remove = *node_sp;
		std::vector<std::pair<std::shared_ptr<Node>, bool>> affected_nodes;
		remove_helper(node_to_remove, affected_nodes);
        /* std::vector<typename Virus::id_type> parent_ids = get_parents(id); // const?
        for (size_t i = 0; i < parent_ids.size(); i++) {
            std::shared_ptr<Node> parent_sp = genealogy_.find(parent_ids[i])->second;
            parent_sp->remove_child(node_sp);
        }

        std::vector<typename Virus::id_type> children_ids = get_children(id);
        for (size_t i = 0; i < children_ids.size(); i++) {
            std::shared_ptr<Node> child_sp = genealogy_.find(children_ids[i])->second;
            child_sp->remove_parent(node_sp);

            if (child_sp->get_parents().size() == 0) {
                remove(child_sp->get_id());
            }
        }

        genealogy_.erase(id); */
        std::cout<<"akdhsjiad\n";
        for (auto affected_node : affected_nodes) {
			auto it = affected_node.first->get_iter();
			Node n = *affected_node.first;
			if (!affected_node.second) {
				// tutaj trzeba jakoś zamienić, pod affected_node.first jest uaktualniony
				// i trzeba go wrzucić na mapę
				//it->second = affected_node.first;
				//n = *genealogy_.find(n.get_id())->second;
			}
			else
				genealogy_.erase(it); 
				
			std::cout<<n.get_virus().get_id()<<": (child)\n";
			for (size_t i = 0; i < n.get_children().size(); i++)
				std::cout<<n.get_children()[i]<<" ";
			std::cout<<"\n";
			
			std::cout<<n.get_virus().get_id()<<": (par)\n";
			auto p = n.get_parents();
			for (auto affected_node : p)
				std::cout<<affected_node<<" ";
			std::cout<<"\n";
		}
    }
};

#endif
