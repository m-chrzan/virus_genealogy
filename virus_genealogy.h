#ifndef VIRUS_GENEALOGY
#define VIRUS_GENEALOGY

#include <vector>
#include <map>
#include <utility>
#include <memory>


class VirusNotFound: std::exception{
	
};

class VirusAlreadyCreated: std::exception{
	
};

class TriedToRemoveStemVirus: std::exception{
	
};

template<class Virus>
class VirusGenealogy{
	private:
		class Node{
			public:
				typename Virus::id_type id_;
                Virus virus_;

				std::vector<typename Virus::id_type> children;
				std::vector<typename Virus::id_type> parents;

				Node(typename Virus::id_type id) : id_(id),
                                                   virus_(Virus(id)) {};
						
				void add_child(typename Virus::id_type const &child_id) {
					children.push_back(child_id); //strong
				}
						
				void add_parent(typename Virus::id_type const &parent_id) {
					parents.push_back(parent_id); // strong
				}
						
				void add_parents(std::vector<typename Virus::id_type> const &parent_ids) {
					// przy założeniu, że konstruktor kopiujący typename Virus::id_type jest no throw
					// bedzie rzucac, trzeba zmodyfikowac
					parents.insert(parents.end(), parent_ids.begin(), parent_ids.end());
				}
				// chyba trzeba dopisać konstruktory i = żeby były nothrow/strong
				// == na virusie tez moze rzucac wyjatek
				// wszystko co moze bedzie rzucac
				// nie kopiowac mapy, mozna w nodzie trzymac jakis iterator wskazujacy miejsce
				// nawet iterowanie po wektorze może rzucić wyjątkiem
				// ale można tutaj trzymać iterator do miejsca w mapie
				// można kopiować parents/children ale dużej mapy nie
		};
		
		std::map<typename Virus::id_type, std::shared_ptr<Node>> mapa;
		typename Virus::id_type first_id;
		
	public:
		// Tworzy nową genealogię.
		// Tworzy także węzeł wirusa macierzystego o identyfikatorze stem_id.
		VirusGenealogy(typename Virus::id_type const &stem_id)
			:first_id(stem_id) {
			mapa.insert(make_pair(stem_id, std::make_shared<Node>(stem_id))); // strong
		};

		// Zwraca identyfikator wirusa macierzystego.
		typename Virus::id_type get_stem_id() const {
			return first_id; // nothrow
		}

		// Zwraca listę identyfikatorów bezpośrednich następników wirusa
		// o podanym identyfikatorze.
		// Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
		std::vector<typename Virus::id_type> get_children(typename Virus::id_type const &id) const {
			if (!exists(id))
				throw VirusNotFound();
				
			std::shared_ptr<Node> ptr = mapa.find(id)->second; // strong
			return ptr->children; // nothrow
			
		}

		// Zwraca listę identyfikatorów bezpośrednich poprzedników wirusa
		// o podanym identyfikatorze.
		// Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
		std::vector<typename Virus::id_type> get_parents(typename Virus::id_type const &id) const {
			if (!exists(id))
				throw VirusNotFound();
				
			std::shared_ptr<Node> ptr = mapa.find(id)->second; // strong
			return ptr->parents; // nothrow?
		}

		// Sprawdza, czy wirus o podanym identyfikatorze istnieje.
		bool exists(typename Virus::id_type const &id) const {
			return mapa.find(id) != mapa.end(); // strong
		}

		// Zwraca referencję do obiektu reprezentującego wirus o podanym
		// identyfikatorze.
		// Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
		Virus& operator[](typename Virus::id_type const &id) const {
            if (!exists(id))
                throw VirusNotFound();

            auto iterator = mapa.find(id);
            std::shared_ptr<Node> node_sp = std::get<1>(*iterator);
            Node& node = *(node_sp.get());
            return node.virus_;
        }

		// Tworzy węzeł reprezentujący nowy wirus o identyfikatorze id
		// powstały z wirusów o podanym identyfikatorze parent_id lub
		// podanych identyfikatorach parent_ids.
		// Zgłasza wyjątek VirusAlreadyCreated, jeśli wirus o identyfikatorze
		// id już istnieje.
		// Zgłasza wyjątek VirusNotFound, jeśli któryś z wyspecyfikowanych
		// poprzedników nie istnieje.
		void create(typename Virus::id_type const &id, typename Virus::id_type const &parent_id) {
			if (exists(id))
				throw VirusAlreadyCreated();
			if (!exists(parent_id))
				throw VirusNotFound();
	
			std::shared_ptr<Node> parent = mapa.find(parent_id)->second; // strong
			std::shared_ptr<Node> node =
				std::make_shared<Node>(id);
			node->add_parent(parent_id); // strong
			mapa.insert(make_pair(id, node)); // strong
			parent->add_child(id); // both strong
		}
		
		void create(typename Virus::id_type const &id, std::vector<typename Virus::id_type> const &parent_ids) {
			if (exists(id))
				throw VirusAlreadyCreated();
				
			for (typename Virus::id_type parent_id : parent_ids)
				if (!exists(parent_id))
					throw VirusNotFound();
				
			std::shared_ptr<Node>
				node = std::make_shared<Node>(id);
			node->add_parents(parent_ids); // strong
			mapa.insert(make_pair(id, node)); // strong
			for (typename Virus::id_type parent_id : parent_ids) {
				std::shared_ptr<Node> parent = mapa.find(parent_id)->second; // strong
				parent->add_child(id); // strong
			}
		}

		// Dodaje nową krawędź w grafie genealogii.
		// Zgłasza wyjątek VirusNotFound, jeśli któryś z podanych wirusów nie istnieje.
		void connect(typename Virus::id_type const &child_id, typename Virus::id_type const &parent_id) {
			if (!exists(parent_id) || !exists(child_id))
				throw VirusNotFound();
			
			std::shared_ptr<Node> parent = mapa.find(parent_id)->second; // strong
			std::shared_ptr<Node> child = mapa.find(child_id)->second; // strong
				
			parent->add_child(child_id); // strong
			child->add_parent(parent_id); // strong
		}

		// Usuwa wirus o podanym identyfikatorze.
		// Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
		// Zgłasza wyjątek TriedToRemoveStemVirus przy próbie usunięcia
		// wirusa macierzystego.
		// set zamiast wektora żeby usuwać szybciej?
		void remove(typename Virus::id_type const &id);
};
#endif
