#include "drone.h"
#include "campo.h"
#include "auxiliares.h"
#include <ostream>

Drone::Drone()
{
}

Drone::Drone(ID i, const std::vector<Producto>& ps)
{
  _id = i;
  setBateria(100);
  _enVuelo = false;
  _productos = ps;
  Posicion p;
  p.x = 0;
  p.y = 0;
  _posicionActual = p;
}

ID Drone::id() const
{
  return _id;
}

Carga Drone::bateria() const
{
  return _bateria;
}

bool Drone::enVuelo() const
{
  return _enVuelo;
}

const Secuencia<Posicion>& Drone::vueloRealizado() const
{
  return _trayectoria;
}

Posicion Drone::posicionActual() const
{
  return _posicionActual;
}

const Secuencia<Producto>& Drone::productosDisponibles() const
{
  return _productos;
}

bool Drone::vueloEscalerado() const
{
  // E0
  bool res = enVuelo();
  // E1
  Secuencia<Posicion> posiciones = vueloRealizado();
  //E2
  if (posiciones.size() > 2 && enVuelo()) {
    //E-IF0
    Posicion distancia1a3 = restarPosiciones(posiciones[0], posiciones[2]);
    //E-IF1
    int i = 0;
    //E-IF3
    while (i < (int) posiciones.size() - 2) {
      //E-C0
      Posicion distanciaDeADos = restarPosiciones(posiciones[i], posiciones[i+2]);
      //E-C1
      res = res && distanciaDeADos == distancia1a3;
      //E-C2
      i++;
      //E-C3
    }
    //E-IF4
    res = res && distancia1a3.x != 0 && distancia1a3.y != 0;
    //E-IF5
  }
  //E3
  return res;
  //E4
}

Secuencia<InfoVueloCruzado> Drone::vuelosCruzados(const Secuencia<Drone>& ds)
{
  Secuencia<InfoVueloCruzado> vuelosCruzados;

  //Usamos unsigned int para evitar warnings al compilar (size() de vector devuelve unsigned int)
  unsigned int n = 0;
  while (n < ds.size()) {
    unsigned int j = 0;
    while (j < ds[n].vueloRealizado().size()) {
      /*Buscamos todos los cruces en las posiciones contenidas en vueloRealizados() de todos
      los drones. Produce un vector de info desordenada y con muchos elementos repetidos.
      Luego de formar el vector hay que encargarse de ordenar y eliminar repetidos*/
      if (cantidadCruces(ds, ds[n].vueloRealizado()[j], ds[n].vueloRealizado().size()) > 1) {
        InfoVueloCruzado info;
        info.posicion = ds[n].vueloRealizado()[j];
        info.cantidadCruces = cantidadCruces(ds, ds[n].vueloRealizado()[j], ds[n].vueloRealizado().size());
        vuelosCruzados.push_back(info);
      }
      j++;
    }
    n++;
  }

  //Ordenamos y sacamos repetidos.
  Secuencia<InfoVueloCruzado> res = elimCruzadosRepetidos(vuelosCruzados);
  ordenarVuelosCruzados(res);
  return res;
}


void Drone::mostrar(std::ostream & os) const
{
  os << "Drone[" << id() <<"] (";
  if (enVuelo()) {
    os << "volando";
  } else {
    os << "no volando";
  }

  os << " - " << bateria() << "%)";

  os << " Productos: [";
  unsigned int n = 0;
  while (n < productosDisponibles().size()) {
    os << productosDisponibles()[n];
    n++;
    if (n < productosDisponibles().size()) {
      os << ", ";
    }
  }
  os << "]";

  os << " Posicion: " << posicionActual() << " - Trayectoria: [";
  unsigned int i = 0;
  while (i < vueloRealizado().size()) {
    os << vueloRealizado()[i];
    i++;
    if (i < vueloRealizado().size()) {
      os << ", ";
    }
  }
  os << "]\n";
}



void Drone::guardar(std::ostream & os) const
{
  os << "{ D " << id() << " " << bateria() << " [";
  unsigned int n = 0;
  while (n < vueloRealizado().size()) {
    os << vueloRealizado()[n];
    n++;
    if (n < vueloRealizado().size()) {
      os << ",";
    }
  }
  os << "] [";
  unsigned int i = 0;
  while (i < productosDisponibles().size()) {
    os << productosDisponibles()[i];
    i++;
    if (i < productosDisponibles().size()) {
      os << ",";
    }
  }
  os << "] ";
  if (enVuelo()) {
    os << "true ";
  } else {
    os << "false ";
  }
  os << posicionActual() << "}";
}

void Drone::cargar(std::istream & is)
{
  std::string raw;
  getline(is, raw, '}');
  Secuencia<std::string> datos = splitBy(raw.substr(1, raw.length()-1), " ");

  _id = atoi(datos[1].c_str());
  setBateria(atoi(datos[2].c_str()));
  if (datos[5].compare("true") == 0) {
    _enVuelo = true;
  } else {
    _enVuelo = false;
  }

  // dato[3] tiene [[1,2],[1,3]] y yo quiero sacar los [] que encierran a todos los datos
  if (datos[3].length() > 2) {
    _trayectoria = damePosiciones(datos[3].substr(1, datos[3].length()-2));
  }
  _posicionActual = damePosiciones(datos[6])[0];
  if (datos[4].length() > 2) {
    _productos = dameProductos(datos[4].substr(1, datos[4].length()-2));
  }
}

void Drone::moverA(const Posicion pos)
{
  _enVuelo = true;
  _trayectoria.push_back(pos);
  _posicionActual.x = pos.x;
  _posicionActual.y = pos.y;


  //cambiarPosicionActual(pos); // por invariante agregamos esta linea
  // debemos verificar invariante movimientoOK? pos tiene que ser un movimiento valido?
}

void Drone::setBateria(const Carga c)
{
  _bateria = c;
}

void Drone::borrarVueloRealizado()
{
  _enVuelo = false;
  _trayectoria = std::vector<Posicion>();
}

void Drone::cambiarPosicionActual(const Posicion p)
{
  _posicionActual.x = p.x;
  _posicionActual.y = p.y;
}

void Drone::sacarProducto(const Producto p)
{
  int indiceProducto;
  unsigned n=0;
  while (n < _productos.size()) {
    if (_productos[n] == p) {
      indiceProducto = n;
    }
    n++;
  }
  _productos.erase(_productos.begin() + indiceProducto);
}

bool Drone::operator==(const Drone & otroDrone) const
{
  bool res = true;
  res = res && id() == otroDrone.id();
  res = res && bateria() == otroDrone.bateria();
  res = res && enVuelo() == otroDrone.enVuelo();
  unsigned int n = 0;
  while (n < vueloRealizado().size()) {
    res = res && vueloRealizado()[n] == otroDrone.vueloRealizado()[n];
    n++;
  }
  res = res && vueloRealizado().size() == otroDrone.vueloRealizado().size();
  res = res && posicionActual() == otroDrone.posicionActual();
  res = res && mismosProductos(productosDisponibles(), otroDrone.productosDisponibles());
  return res;
}

std::ostream & operator<<(std::ostream & os, const Drone & d)
{
  d.mostrar(os);
  return os;
}

/******************** AUX **************************/
bool Drone::mismosProductos(const Secuencia<Producto> lista1, const Secuencia<Producto> lista2) {
  bool res = lista1.size() == lista2.size();
  unsigned int n = 0;
  while (res && n < lista1.size()) {
    res = res && (cantidad(lista1, lista1[n]) == cantidad(lista2, lista1[n]));
    res = res && (cantidad(lista1, lista2[n]) == cantidad(lista2, lista2[n]));
    n++;
  }

  return res;
}

int Drone::cantidad(const Secuencia<Producto> lista, Producto producto) {
  unsigned int n = 0;
  int cant = 0;
  while (n < lista.size()) {
    if (lista[n] == producto) {
      cant++;
    }
    n++;
  }
  return cant;
}

int Drone::cantidadCruces(const Secuencia<Drone>& ds, Posicion pos, int longitud) {
  int n = 0;
  int total = 0;
  while (n < longitud) {
    int cant = 0;
    unsigned int j = 0;
    while (j < ds.size()) {
      if (ds[j].vueloRealizado()[n] == pos) {
        cant++;
      }
      j++;
    }
    /*Si hubo mas de un drone al mismo tiempo en una posicion se considera
    cruce y la cantidad de drones involucrados en el cruce se suma al total
    de drones que se cruzaron en esa posicion (acumula todos los que se
    cruzaron en momentos distintos)*/
    if (cant > 1) {
      total = total + cant;
    }
    cant = 0;
    n++;
  }
  return total;
}

//bubble sort
void Drone::ordenarVuelosCruzados(Secuencia<InfoVueloCruzado>& vuelosCruzados){
  bool intercambiado = true;
  while(intercambiado && vuelosCruzados.size() > 1){
    intercambiado = false;
    int i = 0;
    while(i < vuelosCruzados.size()-1){
      if(vuelosCruzados[i].cantidadCruces > vuelosCruzados[i+1].cantidadCruces){
        vuelosCruzados[i].cantidadCruces += vuelosCruzados[i+1].cantidadCruces;
        vuelosCruzados[i+1].cantidadCruces = vuelosCruzados[i].cantidadCruces - vuelosCruzados[i+1].cantidadCruces;
        vuelosCruzados[i].cantidadCruces -= vuelosCruzados[i+1].cantidadCruces;
        intercambiado = true;
      }
      i++;
    }
  }
}

Secuencia<InfoVueloCruzado> Drone::elimCruzadosRepetidos(const Secuencia<InfoVueloCruzado>& vuelosCruzados){
  /*Creamos una lista nueva porque ir modificando la lista original mientras
  hacemos consultas sobre ella podría traer problemas*/ 
  Secuencia<InfoVueloCruzado> res;

  unsigned int i = 0;
  while(i < vuelosCruzados.size()){
    bool repetido = false;
    unsigned int j = i + 1;
    while(j < vuelosCruzados.size()){
      repetido = repetido || vuelosCruzados[i].posicion == vuelosCruzados[j].posicion;
      j++;
    }
    if(!repetido){
      res.push_back(vuelosCruzados[i]);
    }
    i++;
  }
  return res;
}
