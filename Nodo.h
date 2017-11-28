#pragma once
#include "cocos2d.h"
#include <unordered_map>
USING_NS_CC;

class Nodo {
public:
	float h;
	float g;
	float f;
	bool raiz = false;
	Vec2 estado;
	std::string state;
	Nodo* padre;
	Nodo(){};
	std::unordered_map<std::string, std::vector<std::string>> adyacencias;
	std::vector<Vec2> camino;
	Nodo(std::string state, Vec2 estado, Nodo* padre, float g, float h) : state(state), estado(estado), padre(padre), g(g), h(h), f(g + h) {};
	Nodo make_node(std::string state, Vec2 estado, float g, float h) {
		return Nodo(state, estado, this, g, h);
	};
	float heuristica(Vec2 otro) {
		return sqrt(pow((this->estado.x - otro.x),2) + pow((this->estado.y - otro.y),2));
	};

	bool Nodo::operator<(Nodo const& n1) const {
		return this->f > n1.f;
	};

	Nodo a_estrella(Nodo meta) {
		std::priority_queue<Nodo> abiertos;
		std::unordered_map <std::string, float> cerrados;

		this->h = this->heuristica(meta.estado);
		this->raiz = true;
		abiertos.push(*this);
		this->camino.push_back(this->estado);
		
		while (!abiertos.empty()) {
			
			Nodo actual = abiertos.top();
			abiertos.pop();
			CCLOG("SI EXPANDO");
			CCLOG("%s vs ", actual.state.c_str());
			CCLOG("SI EXPANDO");
			if (actual.state.compare(meta.state) == 0) {
				
				return actual;
			}
			cerrados[actual.state] = actual.g;
			CCLOG("%s vs ", actual.state.c_str());
			
			for (std::vector<std::string>::iterator it = adyacencias[actual.state].begin(); it != adyacencias[actual.state].end(); ++it) {
				/* std::cout << *it; ... */
				CCLOG("SI ENTRO");

				Nodo aux;
				std::string hijo = *it;
				CCLOG("SI ENTRO");
				std::string delimiter = ",";
				std::string token = hijo.substr(0, hijo.find(delimiter));
				Vec2 estado;
				estado.x = atof(token.c_str());
				token = hijo.substr(hijo.find(delimiter) + 1, hijo.length());
				estado.y = atof(token.c_str());

				aux = actual.make_node(hijo, estado, actual.g + 1, 0);
				aux.h = aux.heuristica(meta.estado);
				aux.f = aux.h + aux.g;
				aux.camino = actual.camino;
				aux.camino.push_back(aux.estado);

				CCLOG("%f valor f ", aux.f);
				if (cerrados.find(aux.state) != cerrados.end()) continue;
				CCLOG("%f %f", aux.estado.x, aux.estado.y);
				abiertos.push(aux);
			}
			
			
		}
		return Nodo();
	};

	std::vector<Vec2> camino1() {
		std::vector<Vec2> salida;
		//salida.push_back(nodo);
		Vec2 aux;
		CCLOG("%f %f", this->estado.x, this->estado.y);
		for (std::vector<Vec2>::iterator it = camino.begin(); it != camino.end(); ++it) {
			aux = *it;
			CCLOG("%f %f los nodos", aux.x, aux.y);
			salida.push_back(Vec2(aux.x, aux.y));
		}
		return salida;


		/*while (!aux.raiz) {
			salida.push_back(aux);
			aux = *(aux.padre);
			CCLOG("%f %f", aux.estado.x, aux.estado.y);
		}
		return salida;*/
	
	}


};