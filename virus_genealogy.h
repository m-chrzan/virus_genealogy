#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <cstddef>
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
    
    typedef typename Virus::id_type id_type;
    class Node;

    id_type stem_id_;
    std::map<id_type, std::shared_ptr<Node>> genealogy_;

    void throw_if_not_found(id_type const &id) const {
        if (!exists(id)) {
            throw VirusNotFound();
        }
    }

    void throw_if_already_created(id_type const &id) const {
        if (exists(id)) {
            throw VirusAlreadyCreated();
        }
    }

    class Node {
    private:
    
        typedef typename std::map<id_type, 
                std::shared_ptr<Node>>::iterator map_iter;
                      
        Virus virus_;
        id_type id_;
        map_iter iter_;
        bool pending_remove;

        std::set<std::weak_ptr<Node>, 
                 std::owner_less<std::weak_ptr<Node>>> children_;
        std::set<std::weak_ptr<Node>, 
                 std::owner_less<std::weak_ptr<Node>>> parents_;
        
    public:
                
        Node(id_type const &id) : virus_(id), id_(id) {
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

        std::vector<id_type> get_children() const {
            std::vector<id_type> children_ids;
            for (std::weak_ptr<Node> node_wp : children_) {
                std::shared_ptr<Node> node_sp = node_wp.lock();
                children_ids.push_back(node_sp->id_);
            }

            return children_ids; 
        }

        std::vector<id_type> get_parents() const {
            std::vector<id_type> parent_ids;
            for (std::weak_ptr<Node> node_wp : parents_) {
                std::shared_ptr<Node> node_sp = node_wp.lock();
                parent_ids.push_back(node_sp->id_);
            }

            return parent_ids;
        }
        

        Virus& get_virus() const {
            return virus_;
        }

        id_type get_id() const {
            return id_;
        }
        
        map_iter get_iter() const {
            return iter_;
        }
            
        void set_iter(map_iter it) {
            iter_ = it;
        }
        
        bool get_pending_remove() const {
			return pending_remove;
		}
		
		void set_pending_remove(bool b) {
			pending_remove = b;
		}
    };
    
public:

    VirusGenealogy(id_type const &stem_id) : stem_id_(stem_id) {
        std::shared_ptr<Node> node_sp = std::make_shared<Node>(Node(stem_id));
        auto it = genealogy_.insert(std::make_pair(stem_id, node_sp)).first;
        node_sp->set_iter(it);
    };
    
    VirusGenealogy(VirusGenealogy &) = delete;

    VirusGenealogy& operator=(const VirusGenealogy &other) = delete;
    
    id_type get_stem_id() const {
        return stem_id_;
    }

    std::vector<id_type>
        get_children(id_type const &id) const {
        throw_if_not_found(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second;
        return node_sp->get_children();
    }

    std::vector<id_type> get_parents(id_type const &id) const {
        throw_if_not_found(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second;
        return node_sp->get_parents();
    }

    bool exists(id_type const &id) const {
        return genealogy_.find(id) != genealogy_.end();
    }

    Virus& operator[](id_type const &id) const {
        throw_if_not_found(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second;
        return node_sp->get_virus();
    }

    void create(id_type const &id, id_type const &parent_id) {
		create(id, std::vector<id_type>{parent_id});
    }

    void create(id_type const &id, std::vector<id_type> const &parent_ids) {
        throw_if_already_created(id);
        
        std::vector<std::shared_ptr<Node>> nodes;
        for (std::size_t i = 0; i < parent_ids.size(); i++) {
            throw_if_not_found(parent_ids[i]);
        }
        
         for (std::size_t i = 0; i < parent_ids.size(); i++) {
            nodes.push_back(genealogy_.find(parent_ids[i])->second); 
        }
        
        std::shared_ptr<Node> node_sp = std::make_shared<Node>(id);
        auto it = genealogy_.insert(std::make_pair(id, node_sp)).first;
        
        try {
            node_sp->set_iter(it);
            for (std::size_t i = 0; i < parent_ids.size(); i++) {
                connect(id, parent_ids[i]);
            }
        } catch (...) {
            genealogy_.erase(it);
            for (std::size_t i = 0; i < nodes.size(); i++) {
                nodes[i]->remove_child(node_sp);
                node_sp->remove_parent(nodes[i]);
            }
            throw;
        }
    } // try-catch-reverse makes the whole function strong

    void connect(id_type const &child_id, id_type const &parent_id) {
        throw_if_not_found(child_id);
        throw_if_not_found(parent_id);

        std::shared_ptr<Node> child_sp = genealogy_.find(child_id)->second;

        std::shared_ptr<Node> parent_sp = genealogy_.find(parent_id)->second;
        
        if (!child_sp->edge_exists(parent_sp)) {
            child_sp->add_parent(parent_sp);
            try {
                parent_sp->add_child(child_sp);
            } catch (...) {
                child_sp->remove_parent(parent_sp);
                throw;
            }
        }
    } // try-catch-reverse makes whole function strong
    
    std::vector<std::shared_ptr<Node>> remove_helper(id_type id,
         std::map<id_type, std::shared_ptr<Node>> &gen_tmp, bool first) {	
        auto node_sp = genealogy_.find(id)->second;
        std::vector<id_type> parent_ids = node_sp->get_parents();
        
        for (std::size_t i = 0; i < parent_ids.size(); i++) {
			auto iter = gen_tmp.find(parent_ids[i]);
			std::shared_ptr<Node> parent_sp;
			if (iter != gen_tmp.end())
				parent_sp = iter->second;
			else
				parent_sp = genealogy_.find(parent_ids[i])->second;
            Node copy = *parent_sp;
            copy.remove_child(node_sp);
            gen_tmp[copy.get_id()] = std::make_shared<Node>(copy);
        }

        std::vector<id_type> children_ids = node_sp->get_children();
        
        for (std::size_t i = 0; i < children_ids.size(); i++) {
            auto iter = gen_tmp.find(children_ids[i]);
			std::shared_ptr<Node> child_sp;
			if (iter != gen_tmp.end())
				child_sp = iter->second;
			else
				child_sp = genealogy_.find(children_ids[i])->second;
            Node copy = *child_sp;
            copy.remove_parent(node_sp);
            gen_tmp[copy.get_id()] = std::make_shared<Node>(copy);
            if (copy.get_parents().size() == 0) {
                remove_helper(copy.get_id(), gen_tmp, false);
            }
        }
        
        Node copy = *node_sp;
        copy.set_pending_remove(true);
        gen_tmp[copy.get_id()] = std::make_shared<Node>(copy);
        
        if (first) {
			std::vector<std::shared_ptr<Node>> nodes;
			for (auto pair : gen_tmp) {
				nodes.push_back(pair.second);
			}
			return nodes;
		} else {
			return std::vector<std::shared_ptr<Node>>(0);
		}
    }
        
    void remove(id_type const &id) {
        if (id == stem_id_) {
           throw TriedToRemoveStemVirus();
        }
        throw_if_not_found(id);

        std::shared_ptr<Node> node_sp = genealogy_.find(id)->second;
        Node node_to_remove = *node_sp;
        std::map<id_type, std::shared_ptr<Node>> mapa;
        mapa.insert(std::make_pair(id, std::make_shared<Node>(node_to_remove)));
        std::vector<std::shared_ptr<Node>> affected_nodes = 
                                           remove_helper(id, mapa, true);
        
        for (auto affected_node : affected_nodes) {
            Node node = *affected_node;
            auto it = node.get_iter();
            if (!node.get_pending_remove()) {
                std::swap(*(it->second), node);    
            }
            else {
                genealogy_.erase(it); 
            }
        }
    }
};

#endif
