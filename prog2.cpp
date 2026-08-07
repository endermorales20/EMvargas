#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <limits>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
    #include <direct.h>
    #define CREAR_CARPETA(ruta) _mkdir(ruta)
#else
    #define CREAR_CARPETA(ruta) mkdir(ruta, 0777)
#endif

using namespace std;

// ==========================================
// CONSTANTES Y ESTRUCTURAS OBLIGATORIAS
// ==========================================

const int MAX_RESULTADOS = 100;

// Estructura de cabecera obligatoria de 16 bytes para archivos con múltiples registros
struct ArchivoHeader {
    int cantidadRegistros;  // Total histórico (incluyendo eliminados lógicamente)
    int proximoID;          // Siguiente ID a asignar (autoincremental)
    int registrosActivos;   // Registros con eliminado == false
    int version;            // Control de versión del archivo (inicia en 1)
};

struct Torneo {
    char nombre[100];
    char deporte[50];
    char formato[20];
    char fechaInicio[11];
    char fechaFin[11];
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

struct Equipo {
    int  id;
    char nombre[100];
    char ciudad[100];
    char entrenador[100];

    int  puntos;
    int  victorias;
    int  empates;
    int  derrotas;
    int  golesAFavor;
    int  golesEnContra;

    int  partidosIDs[50];       // Límite de 50 partidos por equipo
    int  cantidadPartidos;

    bool     eliminado;
    time_t   fechaCreacion;
    time_t   fechaUltimaModificacion;
};

struct Jugador {
    int  id;
    int  idEquipo;
    char nombre[100];
    char cedula[20];
    char posicion[20];
    int  edad;
    int  numeroDorsal;

    int  golesAnotados;
    int  tarjetasAmarillas;
    int  tarjetasRojas;

    bool     eliminado;
    time_t   fechaCreacion;
    time_t   fechaUltimaModificacion;
};

struct Gol {
    int  idJugador;       // 0 = Desconocido / Autogol
    int  minuto;          // 1 - 120
    char equipo[12];      // "LOCAL" o "VISITANTE"
};

struct Partido {
    int  id;
    int  idEquipoLocal;
    int  idEquipoVisitante;
    char fecha[11];
    char estado[12];       // "PROGRAMADO", "JUGADO", "CANCELADO"
    char descripcion[200];

    int  golesLocal;
    int  golesVisitante;

    Gol  goles[22];        // Máximo 22 goles por partido
    int  numGoles;

    bool     eliminado;
    time_t   fechaCreacion;
    time_t   fechaUltimaModificacion;
};

// Rutas a los ficheros de datos
const char* PATH_TORNEO = "torneo.bin";
const char* PATH_EQUIPOS = "equipos.bin";
const char* PATH_JUGADORES = "jugadores.bin";
const char* PATH_PARTIDOS = "partidos.bin";

// ==========================================
// CAPA DE PERSISTENCIA - BAJO NIVEL (I/O)
// ==========================================

bool inicializarArchivo(const char* nombreArchivo) {
    // Si es torneo.bin, se inicializa de forma diferente (no requiere ArchivoHeader)
    if (strcmp(nombreArchivo, PATH_TORNEO) == 0) {
        ifstream test(nombreArchivo, ios::binary);
        if (test) {
            test.close();
            return true; // Ya existe
        }
        ofstream creacion(nombreArchivo, ios::binary | ios::trunc);
        if (!creacion) return false;
        
        Torneo t = {};
        strcpy(t.nombre, "Torneo Universitario URU");
        strcpy(t.deporte, "Futbol");
        strcpy(t.formato, "Liga");
        strcpy(t.fechaInicio, "2026-06-01");
        strcpy(t.fechaFin, "2026-07-15");
        t.fechaCreacion = time(nullptr);
        t.fechaUltimaModificacion = time(nullptr);
        
        creacion.write(reinterpret_cast<char*>(&t), sizeof(Torneo));
        creacion.close();
        return true;
    }

    // Para el resto de los archivos con registros múltiples
    ifstream test(nombreArchivo, ios::binary);
    if (test) {
        test.close();
        return true; // Ya existe
    }

    ofstream creacion(nombreArchivo, ios::binary | ios::trunc);
    if (!creacion) return false;

    ArchivoHeader header = {0, 1, 0, 1}; // Inicia con 0 registros, próximo ID 1, activos 0, versión 1
    creacion.write(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    creacion.close();
    return true;
}

bool inicializarSistemaArchivos() {
    // Creación de la carpeta "datos" no es estrictamente obligatoria si trabajamos local, 
    // pero inicializamos los archivos directamente para garantizar portabilidad de rutas relativas.
    return inicializarArchivo(PATH_TORNEO) &&
           inicializarArchivo(PATH_EQUIPOS) &&
           inicializarArchivo(PATH_JUGADORES) &&
           inicializarArchivo(PATH_PARTIDOS);
}

ArchivoHeader leerHeader(const char* nombreArchivo) {
    ArchivoHeader header = {0, 1, 0, 1};
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo) return header;
    
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    archivo.close();
    return header;
}

bool actualizarHeader(const char* nombreArchivo, ArchivoHeader header) {
    fstream archivo(nombreArchivo, ios::binary | ios::in | ios::out);
    if (!archivo) return false;
    
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

// Búsqueda secuencial pero cargando UN solo registro a la vez en memoria RAM
int buscarIndicePorID(const char* nombreArchivo, int id, int tamanoRegistro) {
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo) return -1;

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    char* buffer = new char[tamanoRegistro];
    int indiceEncontrado = -1;

    for (int i = 0; i < header.cantidadRegistros; i++) {
        // Desplazamiento matemático exacto para situar la aguja de lectura
        // Offset: sizeof(Header) + (i * sizeof(Estructura))
        archivo.seekg(sizeof(ArchivoHeader) + (i * tamanoRegistro), ios::beg);
        archivo.read(buffer, tamanoRegistro);

        // Los primeros 4 bytes de Equipo, Jugador y Partido corresponden siempre al entero 'id'
        int recordId = *reinterpret_cast<int*>(buffer);
        // El campo 'eliminado' se sitúa en posiciones diferentes, por lo que usamos casting directo según tipo de archivo
        bool isEliminado = false;
        if (strcmp(nombreArchivo, PATH_EQUIPOS) == 0) {
            isEliminado = reinterpret_cast<Equipo*>(buffer)->eliminado;
        } else if (strcmp(nombreArchivo, PATH_JUGADORES) == 0) {
            isEliminado = reinterpret_cast<Jugador*>(buffer)->eliminado;
        } else if (strcmp(nombreArchivo, PATH_PARTIDOS) == 0) {
            isEliminado = reinterpret_cast<Partido*>(buffer)->eliminado;
        }

        if (recordId == id && !isEliminado) {
            indiceEncontrado = i;
            break;
        }
    }

    delete[] buffer;
    archivo.close();
    return indiceEncontrado;
}

// ==========================================
// CAPA DE PERSISTENCIA - EQUIPOS
// ==========================================

bool guardarEquipo(Equipo& equipo) {
    ArchivoHeader header = leerHeader(PATH_EQUIPOS);
    
    equipo.id = header.proximoID;
    equipo.eliminado = false;
    equipo.fechaCreacion = time(nullptr);
    equipo.fechaUltimaModificacion = time(nullptr);
    equipo.cantidadPartidos = 0;

    fstream archivo(PATH_EQUIPOS, ios::binary | ios::in | ios::out);
    if (!archivo) return false;

    // Posicionar cursor de escritura al final del archivo para agregar registro nuevo
    archivo.seekp(0, ios::end);
    archivo.write(reinterpret_cast<char*>(&equipo), sizeof(Equipo));
    archivo.close();

    header.cantidadRegistros++;
    header.registrosActivos++;
    header.proximoID++;
    
    return actualizarHeader(PATH_EQUIPOS, header);
}

bool leerEquipoPorID(int id, Equipo& resultado) {
    int indice = buscarIndicePorID(PATH_EQUIPOS, id, sizeof(Equipo));
    if (indice == -1) return false;

    ifstream archivo(PATH_EQUIPOS, ios::binary);
    if (!archivo) return false;

    // Cálculo explícito de offset en bytes (Demostración de Caso de Uso 2)
    streampos offset = sizeof(ArchivoHeader) + (indice * sizeof(Equipo));
    archivo.seekg(offset, ios::beg);
    archivo.read(reinterpret_cast<char*>(&resultado), sizeof(Equipo));
    archivo.close();

    return true;
}

bool actualizarEquipo(Equipo& equipo) {
    int indice = buscarIndicePorID(PATH_EQUIPOS, equipo.id, sizeof(Equipo));
    if (indice == -1) return false;

    fstream archivo(PATH_EQUIPOS, ios::binary | ios::in | ios::out);
    if (!archivo) return false;

    equipo.fechaUltimaModificacion = time(nullptr);

    streampos offset = sizeof(ArchivoHeader) + (indice * sizeof(Equipo));
    archivo.seekp(offset, ios::beg);
    archivo.write(reinterpret_cast<char*>(&equipo), sizeof(Equipo));
    archivo.close();

    return true;
}

bool eliminarEquipoLogico(int id) {
    Equipo eq;
    if (!leerEquipoPorID(id, eq)) return false;

    eq.eliminado = true;
    if (!actualizarEquipo(eq)) return false;

    ArchivoHeader header = leerHeader(PATH_EQUIPOS);
    header.registrosActivos--;
    return actualizarHeader(PATH_EQUIPOS, header);
}

int contarEquiposActivos() {
    ArchivoHeader header = leerHeader(PATH_EQUIPOS);
    return header.registrosActivos;
}

// ==========================================
// CAPA DE PERSISTENCIA - JUGADORES
// ==========================================

bool guardarJugador(Jugador& jugador) {
    ArchivoHeader header = leerHeader(PATH_JUGADORES);

    jugador.id = header.proximoID;
    jugador.eliminado = false;
    jugador.fechaCreacion = time(nullptr);
    jugador.fechaUltimaModificacion = time(nullptr);
    jugador.golesAnotados = 0;
    jugador.tarjetasAmarillas = 0;
    jugador.tarjetasRojas = 0;

    fstream archivo(PATH_JUGADORES, ios::binary | ios::in | ios::out);
    if (!archivo) return false;

    archivo.seekp(0, ios::end);
    archivo.write(reinterpret_cast<char*>(&jugador), sizeof(Jugador));
    archivo.close();

    header.cantidadRegistros++;
    header.registrosActivos++;
    header.proximoID++;

    return actualizarHeader(PATH_JUGADORES, header);
}

bool leerJugadorPorID(int id, Jugador& resultado) {
    int indice = buscarIndicePorID(PATH_JUGADORES, id, sizeof(Jugador));
    if (indice == -1) return false;

    ifstream archivo(PATH_JUGADORES, ios::binary);
    if (!archivo) return false;

    streampos offset = sizeof(ArchivoHeader) + (indice * sizeof(Jugador));
    archivo.seekg(offset, ios::beg);
    archivo.read(reinterpret_cast<char*>(&resultado), sizeof(Jugador));
    archivo.close();

    return true;
}

bool actualizarJugador(Jugador& jugador) {
    int indice = buscarIndicePorID(PATH_JUGADORES, jugador.id, sizeof(Jugador));
    if (indice == -1) return false;

    fstream archivo(PATH_JUGADORES, ios::binary | ios::in | ios::out);
    if (!archivo) return false;

    jugador.fechaUltimaModificacion = time(nullptr);

    streampos offset = sizeof(ArchivoHeader) + (indice * sizeof(Jugador));
    archivo.seekp(offset, ios::beg);
    archivo.write(reinterpret_cast<char*>(&jugador), sizeof(Jugador));
    archivo.close();

    return true;
}

bool eliminarJugadorLogico(int id) {
    Jugador jug;
    if (!leerJugadorPorID(id, jug)) return false;

    jug.eliminado = true;
    if (!actualizarJugador(jug)) return false;

    ArchivoHeader header = leerHeader(PATH_JUGADORES);
    header.registrosActivos--;
    return actualizarHeader(PATH_JUGADORES, header);
}

int contarJugadoresActivos() {
    ArchivoHeader header = leerHeader(PATH_JUGADORES);
    return header.registrosActivos;
}

// ==========================================
// CAPA DE PERSISTENCIA - PARTIDOS
// ==========================================

bool guardarPartido(Partido& partido) {
    ArchivoHeader header = leerHeader(PATH_PARTIDOS);

    partido.id = header.proximoID;
    partido.eliminado = false;
    partido.fechaCreacion = time(nullptr);
    partido.fechaUltimaModificacion = time(nullptr);
    partido.numGoles = 0;
    partido.golesLocal = 0;
    partido.golesVisitante = 0;
    strcpy(partido.estado, "PROGRAMADO");

    fstream archivo(PATH_PARTIDOS, ios::binary | ios::in | ios::out);
    if (!archivo) return false;

    archivo.seekp(0, ios::end);
    archivo.write(reinterpret_cast<char*>(&partido), sizeof(Partido));
    archivo.close();

    header.cantidadRegistros++;
    header.registrosActivos++;
    header.proximoID++;

    return actualizarHeader(PATH_PARTIDOS, header);
}

bool leerPartidoPorID(int id, Partido& resultado) {
    int indice = buscarIndicePorID(PATH_PARTIDOS, id, sizeof(Partido));
    if (indice == -1) return false;

    ifstream archivo(PATH_PARTIDOS, ios::binary);
    if (!archivo) return false;

    streampos offset = sizeof(ArchivoHeader) + (indice * sizeof(Partido));
    archivo.seekg(offset, ios::beg);
    archivo.read(reinterpret_cast<char*>(&resultado), sizeof(Partido));
    archivo.close();

    return true;
}

bool actualizarPartido(Partido& partido) {
    int indice = buscarIndicePorID(PATH_PARTIDOS, partido.id, sizeof(Partido));
    if (indice == -1) return false;

    fstream archivo(PATH_PARTIDOS, ios::binary | ios::in | ios::out);
    if (!archivo) return false;

    partido.fechaUltimaModificacion = time(nullptr);

    streampos offset = sizeof(ArchivoHeader) + (indice * sizeof(Partido));
    archivo.seekp(offset, ios::beg);
    archivo.write(reinterpret_cast<char*>(&partido), sizeof(Partido));
    archivo.close();

    return true;
}

bool eliminarPartidoLogico(int id) {
    Partido part;
    if (!leerPartidoPorID(id, part)) return false;

    part.eliminado = true;
    if (!actualizarPartido(part)) return false;

    ArchivoHeader header = leerHeader(PATH_PARTIDOS);
    header.registrosActivos--;
    return actualizarHeader(PATH_PARTIDOS, header);
}

int contarPartidosActivos() {
    ArchivoHeader header = leerHeader(PATH_PARTIDOS);
    return header.registrosActivos;
}

// ==========================================
// CAPA DE PERSISTENCIA - TORNEO (Registro único)
// ==========================================

bool leerTorneo(Torneo& t) {
    ifstream archivo(PATH_TORNEO, ios::binary);
    if (!archivo) return false;
    archivo.read(reinterpret_cast<char*>(&t), sizeof(Torneo));
    archivo.close();
    return true;
}

bool guardarTorneo(Torneo& t) {
    ofstream archivo(PATH_TORNEO, ios::binary | ios::trunc);
    if (!archivo) return false;
    t.fechaUltimaModificacion = time(nullptr);
    archivo.write(reinterpret_cast<char*>(&t), sizeof(Torneo));
    archivo.close();
    return true;
}

// ==========================================
// FUNCIONES DE BÚSQUEDA / FILTRADO ADAPTADAS (Sin Arrays Dinámicos)
// ==========================================

bool contieneSubcadena(const char* cadena, const char* subcadena) {
    if (!cadena || !subcadena) return false;
    if (subcadena[0] == '\0') return true;
    for (int i = 0; cadena[i] != '\0'; i++) {
        int j = 0;
        while (cadena[i + j] != '\0' && subcadena[j] != '\0' && 
               tolower(cadena[i + j]) == tolower(subcadena[j])) {
            j++;
        }
        if (subcadena[j] == '\0') return true;
    }
    return false;
}

int buscarEquiposPorNombre(const char* subcadena, Equipo resultados[], int maxResultados) {
    ifstream archivo(PATH_EQUIPOS, ios::binary);
    if (!archivo) return 0;

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    int count = 0;
    Equipo temp;
    for (int i = 0; i < header.cantidadRegistros && count < maxResultados; i++) {
        archivo.seekg(sizeof(ArchivoHeader) + (i * sizeof(Equipo)), ios::beg);
        archivo.read(reinterpret_cast<char*>(&temp), sizeof(Equipo));

        if (!temp.eliminado && contieneSubcadena(temp.nombre, subcadena)) {
            resultados[count++] = temp;
        }
    }
    archivo.close();
    return count;
}

int listarJugadoresPorEquipo(int idEquipo, Jugador resultados[], int maxResultados) {
    ifstream archivo(PATH_JUGADORES, ios::binary);
    if (!archivo) return 0;

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    int count = 0;
    Jugador temp;
    for (int i = 0; i < header.cantidadRegistros && count < maxResultados; i++) {
        archivo.seekg(sizeof(ArchivoHeader) + (i * sizeof(Jugador)), ios::beg);
        archivo.read(reinterpret_cast<char*>(&temp), sizeof(Jugador));

        if (!temp.eliminado && temp.idEquipo == idEquipo) {
            resultados[count++] = temp;
        }
    }
    archivo.close();
    return count;
}

int listarPartidosPorEquipo(int idEquipo, Partido resultados[], int maxResultados) {
    ifstream archivo(PATH_PARTIDOS, ios::binary);
    if (!archivo) return 0;

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    int count = 0;
    Partido temp;
    for (int i = 0; i < header.cantidadRegistros && count < maxResultados; i++) {
        archivo.seekg(sizeof(ArchivoHeader) + (i * sizeof(Partido)), ios::beg);
        archivo.read(reinterpret_cast<char*>(&temp), sizeof(Partido));

        if (!temp.eliminado && (temp.idEquipoLocal == idEquipo || temp.idEquipoVisitante == idEquipo)) {
            resultados[count++] = temp;
        }
    }
    archivo.close();
    return count;
}

int listarPartidosPorEstado(const char* estado, Partido resultados[], int maxResultados) {
    ifstream archivo(PATH_PARTIDOS, ios::binary);
    if (!archivo) return 0;

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    int count = 0;
    Partido temp;
    for (int i = 0; i < header.cantidadRegistros && count < maxResultados; i++) {
        archivo.seekg(sizeof(ArchivoHeader) + (i * sizeof(Partido)), ios::beg);
        archivo.read(reinterpret_cast<char*>(&temp), sizeof(Partido));

        if (!temp.eliminado && strcmp(temp.estado, estado) == 0) {
            resultados[count++] = temp;
        }
    }
    archivo.close();
    return count;
}

// ==========================================
// CAPA LÓGICA COMPUESTA: TRANSACCIONES DE PARTIDOS
// ==========================================

bool registrarResultadoPartido(int idPartido, int golesL, int golesV, Gol golesDetalle[], int totalGoles) {
    // 1. Leer Partido desde partidos.bin -> validar que existe y está en "PROGRAMADO"
    Partido part;
    if (!leerPartidoPorID(idPartido, part)) return false;
    if (strcmp(part.estado, "PROGRAMADO") != 0) return false;

    // 2. Leer Equipo Local y Visitante
    Equipo local, visitante;
    if (!leerEquipoPorID(part.idEquipoLocal, local)) return false;
    if (!leerEquipoPorID(part.idEquipoVisitante, visitante)) return false;

    // Sincronizar límites de partidos jugados en la relación interna de los structs
    if (local.cantidadPartidos >= 50 || visitante.cantidadPartidos >= 50) {
        return false;
    }

    // 3. Determinar el resultado y actualizar estadísticas de ambos equipos
    local.golesAFavor += golesL;
    local.golesEnContra += golesV;
    visitante.golesAFavor += golesV;
    visitante.golesEnContra += golesL;

    if (golesL > golesV) {
        local.victorias++;
        local.puntos += 3;
        visitante.derrotas++;
    } else if (golesL < golesV) {
        visitante.victorias++;
        visitante.puntos += 3;
        local.derrotas++;
    } else {
        local.empates++;
        local.puntos += 1;
        visitante.empates++;
        visitante.puntos += 1;
    }

    // 4. Agregar el ID del partido al arreglo partidosIDs[] de cada equipo
    local.partidosIDs[local.cantidadPartidos++] = part.id;
    visitante.partidosIDs[visitante.cantidadPartidos++] = part.id;

    // 5. Actualizar estado del Partido a "JUGADO", registrar goles y detalle
    strcpy(part.estado, "JUGADO");
    part.golesLocal = golesL;
    part.golesVisitante = golesV;
    part.numGoles = totalGoles;
    for (int i = 0; i < totalGoles; i++) {
        part.goles[i] = golesDetalle[i];

        // 6. Actualizar las estadísticas de los jugadores anotadores en jugadores.bin
        if (golesDetalle[i].idJugador > 0) {
            Jugador jug;
            if (leerJugadorPorID(golesDetalle[i].idJugador, jug)) {
                jug.golesAnotados++;
                actualizarJugador(jug);
            }
        }
    }

    // 7. Guardar todos los cambios atómicos en cascada asegurando persistencia inmediata
    if (!actualizarPartido(part)) return false;
    if (!actualizarEquipo(local)) return false;
    if (!actualizarEquipo(visitante)) return false;

    return true;
}

bool cancelarPartidoJugado(int idPartido) {
    // 1. Leer el Partido -> validar que existe y está en "JUGADO"
    Partido part;
    if (!leerPartidoPorID(idPartido, part)) return false;
    if (strcmp(part.estado, "JUGADO") != 0) return false;

    // 2. Leer Equipo Local y Visitante
    Equipo local, visitante;
    if (!leerEquipoPorID(part.idEquipoLocal, local)) return false;
    if (!leerEquipoPorID(part.idEquipoVisitante, visitante)) return false;

    // 3. Revertir estadísticas del torneo de ambos equipos
    local.golesAFavor -= part.golesLocal;
    local.golesEnContra -= part.golesVisitante;
    visitante.golesAFavor -= part.golesVisitante;
    visitante.golesEnContra -= part.golesLocal;

    if (part.golesLocal > part.golesVisitante) {
        local.victorias--;
        local.puntos -= 3;
        visitante.derrotas--;
    } else if (part.golesLocal < part.golesVisitante) {
        visitante.victorias--;
        visitante.puntos -= 3;
        local.derrotas--;
    } else {
        local.empates--;
        local.puntos -= 1;
        visitante.empates--;
        visitante.puntos -= 1;
    }

    // Remover el ID del partido del arreglo interno de partidos de cada equipo
    for (int i = 0; i < local.cantidadPartidos; i++) {
        if (local.partidosIDs[i] == part.id) {
            for (int j = i; j < local.cantidadPartidos - 1; j++) {
                local.partidosIDs[j] = local.partidosIDs[j + 1];
            }
            local.cantidadPartidos--;
            break;
        }
    }
    for (int i = 0; i < visitante.cantidadPartidos; i++) {
        if (visitante.partidosIDs[i] == part.id) {
            for (int j = i; j < visitante.cantidadPartidos - 1; j++) {
                visitante.partidosIDs[j] = visitante.partidosIDs[j + 1];
            }
            visitante.cantidadPartidos--;
            break;
        }
    }

    // 4. Revertir goles de los jugadores anotadores registrados en partido.goles[]
    for (int i = 0; i < part.numGoles; i++) {
        if (part.goles[i].idJugador > 0) {
            Jugador jug;
            if (leerJugadorPorID(part.goles[i].idJugador, jug)) {
                if (jug.golesAnotados > 0) jug.golesAnotados--;
                actualizarJugador(jug);
            }
        }
    }

    // 5. Cambiar estado a "CANCELADO" y limpiar contador de goles
    strcpy(part.estado, "CANCELADO");
    part.numGoles = 0;
    part.golesLocal = 0;
    part.golesVisitante = 0;

    // 6. Escribir los 3 registros actualizados en sus respectivos archivos
    if (!actualizarPartido(part)) return false;
    if (!actualizarEquipo(local)) return false;
    if (!actualizarEquipo(visitante)) return false;

    return true;
}

// ==========================================
// CAPA MANTENIMIENTO E INTEGRIDAD (NUEVAS)
// ==========================================

bool verificarIntegridadReferencial(int& eqVerificados, int& jugVerificados, int& partVerificados, int& rotasEncontradas) {
    eqVerificados = 0;
    jugVerificados = 0;
    partVerificados = 0;
    rotasEncontradas = 0;

    // a) Recorrer jugadores.bin
    ifstream archJug(PATH_JUGADORES, ios::binary);
    if (archJug) {
        ArchivoHeader headerJug;
        archJug.read(reinterpret_cast<char*>(&headerJug), sizeof(ArchivoHeader));
        Jugador tempJug;
        for (int i = 0; i < headerJug.cantidadRegistros; i++) {
            archJug.seekg(sizeof(ArchivoHeader) + (i * sizeof(Jugador)), ios::beg);
            archJug.read(reinterpret_cast<char*>(&tempJug), sizeof(Jugador));
            if (!tempJug.eliminado) {
                jugVerificados++;
                Equipo eqRef;
                if (!leerEquipoPorID(tempJug.idEquipo, eqRef)) {
                    cout << "   -> ROTO: Jugador ID " << tempJug.id << " (" << tempJug.nombre << ") apunta a Equipo ID " << tempJug.idEquipo << " (No existe o eliminado)\n";
                    rotasEncontradas++;
                }
            }
        }
        archJug.close();
    }

    // b) Recorrer partidos.bin
    ifstream archPart(PATH_PARTIDOS, ios::binary);
    if (archPart) {
        ArchivoHeader headerPart;
        archPart.read(reinterpret_cast<char*>(&headerPart), sizeof(ArchivoHeader));
        Partido tempPart;
        for (int i = 0; i < headerPart.cantidadRegistros; i++) {
            archPart.seekg(sizeof(ArchivoHeader) + (i * sizeof(Partido)), ios::beg);
            archPart.read(reinterpret_cast<char*>(&tempPart), sizeof(Partido));
            if (!tempPart.eliminado) {
                partVerificados++;
                Equipo eqLocal, eqVisitante;
                if (!leerEquipoPorID(tempPart.idEquipoLocal, eqLocal)) {
                    cout << "   -> ROTO: Partido ID " << tempPart.id << " apunta a Equipo Local ID " << tempPart.idEquipoLocal << " (No existe o eliminado)\n";
                    rotasEncontradas++;
                }
                if (!leerEquipoPorID(tempPart.idEquipoVisitante, eqVisitante)) {
                    cout << "   -> ROTO: Partido ID " << tempPart.id << " apunta a Equipo Visitante ID " << tempPart.idEquipoVisitante << " (No existe o eliminado)\n";
                    rotasEncontradas++;
                }

                // c) Goles del partido
                for (int g = 0; g < tempPart.numGoles; g++) {
                    int idJug = tempPart.goles[g].idJugador;
                    if (idJug > 0) {
                        Jugador jugRef;
                        if (!leerJugadorPorID(idJug, jugRef)) {
                            cout << "   -> ROTO: Gol en Partido ID " << tempPart.id << " apunta a Jugador ID " << idJug << " (No existe o eliminado)\n";
                            rotasEncontradas++;
                        }
                    }
                }
            }
        }
        archPart.close();
    }

    // Contabilizar equipos verificados de manera informativa
    ArchivoHeader headerEq = leerHeader(PATH_EQUIPOS);
    eqVerificados = headerEq.registrosActivos;

    return true;
}

bool copiarArchivoByteAByte(const char* origen, const char* destino) {
    ifstream f_origen(origen, ios::binary);
    if (!f_origen) return false;
    ofstream f_destino(destino, ios::binary | ios::trunc);
    if (!f_destino) {
        f_origen.close();
        return false;
    }

    char buffer[1024];
    while (f_origen.read(buffer, sizeof(buffer))) {
        f_destino.write(buffer, f_origen.gcount());
    }
    f_destino.write(buffer, f_origen.gcount()); // Escribir el remanente final

    f_origen.close();
    f_destino.close();
    return true;
}

bool crearBackup(char rutaCarpetaOut[150]) {
    time_t ahora = time(nullptr);
    tm* ltm = localtime(&ahora);

    // Formato de nombre: backup_YYYY-MM-DD_HH-MM-SS
    char nombreCarpeta[100];
    sprintf(nombreCarpeta, "backup_%04d-%02d-%02d_%02d-%02d-%02d",
            1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
            ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

    if (CREAR_CARPETA(nombreCarpeta) != 0) {
        return false;
    }

    char destTorneo[150], destEquipos[150], destJugadores[150], destPartidos[150];
    sprintf(destTorneo, "%s/%s", nombreCarpeta, PATH_TORNEO);
    sprintf(destEquipos, "%s/%s", nombreCarpeta, PATH_EQUIPOS);
    sprintf(destJugadores, "%s/%s", nombreCarpeta, PATH_JUGADORES);
    sprintf(destPartidos, "%s/%s", nombreCarpeta, PATH_PARTIDOS);

    bool ok = copiarArchivoByteAByte(PATH_TORNEO, destTorneo) &&
              copiarArchivoByteAByte(PATH_EQUIPOS, destEquipos) &&
              copiarArchivoByteAByte(PATH_JUGADORES, destJugadores) &&
              copiarArchivoByteAByte(PATH_PARTIDOS, destPartidos);

    if (ok) {
        strcpy(rutaCarpetaOut, nombreCarpeta);
    }
    return ok;
}

// ==========================================
// CAPA DE PRESENTACIÓN - REPORTES DESDE DISCO
// ==========================================

void mostrarTablaPosiciones() {
    ArchivoHeader hEq = leerHeader(PATH_EQUIPOS);
    if (hEq.registrosActivos == 0) {
        cout << "No hay equipos activos registrados.\n";
        return;
    }

    // Cargamos un buffer temporal estático sólo para los activos en este instante
    Equipo* tabla = new Equipo[hEq.registrosActivos];
    int count = 0;

    ifstream archivo(PATH_EQUIPOS, ios::binary);
    if (!archivo) {
        delete[] tabla;
        return;
    }
    archivo.seekg(sizeof(ArchivoHeader), ios::beg);
    Equipo temp;
    for (int i = 0; i < hEq.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&temp), sizeof(Equipo));
        if (!temp.eliminado) {
            tabla[count++] = temp;
        }
    }
    archivo.close();

    // Ordenamiento burbuja bajo criterios estipulados
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            bool intercambiar = false;
            if (tabla[j].puntos < tabla[j+1].puntos) {
                intercambiar = true;
            } else if (tabla[j].puntos == tabla[j+1].puntos) {
                int difJ = tabla[j].golesAFavor - tabla[j].golesEnContra;
                int difJ1 = tabla[j+1].golesAFavor - tabla[j+1].golesEnContra;
                if (difJ < difJ1) {
                    intercambiar = true;
                } else if (difJ == difJ1) {
                    if (tabla[j].golesAFavor < tabla[j+1].golesAFavor) {
                        intercambiar = true;
                    }
                }
            }
            if (intercambiar) {
                Equipo tempSwap = tabla[j];
                tabla[j] = tabla[j+1];
                tabla[j+1] = tempSwap;
            }
        }
    }

    Torneo t;
    leerTorneo(t);

    cout << "========================================================================\n";
    cout << "                    TABLA DE POSICIONES - " << t.nombre << "\n";
    cout << "========================================================================\n";
    cout << " # | Equipo               | PTS | PJ | G | E | D | GF | GC | DG\n";
    cout << "---+----------------------+-----+----+---+---+---+----+----+----\n";
    for (int i = 0; i < count; i++) {
        int pj = tabla[i].victorias + tabla[i].empates + tabla[i].derrotas;
        int dg = tabla[i].golesAFavor - tabla[i].golesEnContra;
        printf("%2d | %-20s | %3d | %2d | %1d | %1d | %1d | %2d | %2d | %2d\n",
               i+1, tabla[i].nombre, tabla[i].puntos, pj,
               tabla[i].victorias, tabla[i].empates, tabla[i].derrotas,
               tabla[i].golesAFavor, tabla[i].golesEnContra, dg);
    }
    cout << "========================================================================\n";

    delete[] tabla;
}

void mostrarTablaGoleadores() {
    ArchivoHeader hJug = leerHeader(PATH_JUGADORES);
    if (hJug.registrosActivos == 0) {
        cout << "No hay jugadores activos registrados.\n";
        return;
    }

    Jugador* goleadores = new Jugador[hJug.registrosActivos];
    int count = 0;

    ifstream archivo(PATH_JUGADORES, ios::binary);
    if (!archivo) {
        delete[] goleadores;
        return;
    }
    archivo.seekg(sizeof(ArchivoHeader), ios::beg);
    Jugador temp;
    for (int i = 0; i < hJug.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&temp), sizeof(Jugador));
        if (!temp.eliminado) {
            goleadores[count++] = temp;
        }
    }
    archivo.close();

    // Ordenar descendente por golesAnotados
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (goleadores[j].golesAnotados < goleadores[j+1].golesAnotados) {
                Jugador aux = goleadores[j];
                goleadores[j] = goleadores[j+1];
                goleadores[j+1] = aux;
            }
        }
    }

    int topMax = (count < 10) ? count : 10;

    cout << "========================================================\n";
    cout << "               TABLA DE GOLEADORES (TOP 10)             \n";
    cout << "========================================================\n";
    cout << " # | Jugador                 | Equipo               | Goles\n";
    cout << "---+-------------------------+----------------------+-------\n";
    for (int i = 0; i < topMax; i++) {
        Equipo eq;
        char nombreEq[100];
        if (leerEquipoPorID(goleadores[i].idEquipo, eq)) {
            strcpy(nombreEq, eq.nombre);
        } else {
            strcpy(nombreEq, "Equipo Desconocido");
        }
        printf("%2d | %-23s | %-20s | %3d\n",
               i+1, goleadores[i].nombre, nombreEq, goleadores[i].golesAnotados);
    }
    cout << "========================================================\n";

    delete[] goleadores;
}

void mostrarFichaTecnicaPartido(int idPartido) {
    Partido part;
    if (!leerPartidoPorID(idPartido, part)) {
        cout << "ERROR: Partido con ID " << idPartido << " no encontrado o eliminado.\n";
        return;
    }

    Equipo local, visitante;
    char nameL[100], nameV[100];
    if (leerEquipoPorID(part.idEquipoLocal, local)) strcpy(nameL, local.nombre);
    else strcpy(nameL, "Equipo Local Desconocido");

    if (leerEquipoPorID(part.idEquipoVisitante, visitante)) strcpy(nameV, visitante.nombre);
    else strcpy(nameV, "Equipo Visitante Desconocido");

    cout << "========================================================================\n";
    cout << "                        FICHA TECNICA DEL PARTIDO                       \n";
    cout << "========================================================================\n";
    cout << "  Partido ID  : " << part.id << "          Fecha: " << part.fecha << "\n";
    cout << "  Estado      : " << part.estado << "\n";
    cout << "  Descripcion : " << part.descripcion << "\n";
    cout << "------------------------------------------------------------------------\n";
    cout << "  " << nameL << "   " << part.golesLocal << "  -  " << part.golesVisitante << "   " << nameV << "\n";
    cout << "------------------------------------------------------------------------\n";
    cout << "  DETALLE DE GOLES:\n";
    if (part.numGoles == 0) {
        cout << "  (No se registraron goles anotados o el partido sigue programado)\n";
    } else {
        for (int i = 0; i < part.numGoles; i++) {
            char jugadorNombre[100];
            if (part.goles[i].idJugador > 0) {
                Jugador jug;
                if (leerJugadorPorID(part.goles[i].idJugador, jug)) {
                    strcpy(jugadorNombre, jug.nombre);
                } else {
                    strcpy(jugadorNombre, "Jugador Desconocido");
                }
            } else {
                strcpy(jugadorNombre, "Autogol / Desconocido");
            }
            printf("  [%s] Min. %2d - %s\n", part.goles[i].equipo, part.goles[i].minuto, jugadorNombre);
        }
    }
    cout << "========================================================================\n";
}

// ==========================================
// CAPA DE PRESENTACIÓN - MENÚS Y UTILERÍAS
// ==========================================

void leerEntero(int &variable) {
    while (!(cin >> variable)) {
        cout << "Entrada invalida. Ingrese un numero entero: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool confirmarAccion(const char* mensaje) {
    char op;
    cout << mensaje << " (S/N): ";
    cin >> op;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return (op == 'S' || op == 's');
}

void menuEquipos() {
    int opcion;
    do {
        system("cls");
        cout << "==========================================\n";
        cout << "           GESTION DE EQUIPOS             \n";
        cout << "==========================================\n";
        cout << "1. Registrar Equipo\n";
        cout << "2. Buscar Equipo por ID (Acceso Fijo)\n";
        cout << "3. Buscar Equipos por Nombre Parcial\n";
        cout << "4. Actualizar Equipo\n";
        cout << "5. Eliminar Equipo (Borrado Logico)\n";
        cout << "6. Listar todos los Equipos Activos\n";
        cout << "0. Volver al Menu Principal\n";
        cout << "==========================================\n";
        cout << "Seleccione una opcion: ";
        leerEntero(opcion);

        switch(opcion) {
            case 1: {
                Equipo eq = {};
                cout << "Nombre del equipo: ";
                cin.getline(eq.nombre, 99);
                cout << "Ciudad de procedencia: ";
                cin.getline(eq.ciudad, 99);
                cout << "Nombre del Entrenador: ";
                cin.getline(eq.entrenador, 99);

                if (guardarEquipo(eq)) {
                    cout << "Equipo registrado con exito! Asignado ID: " << eq.id << "\n";
                } else {
                    cout << "Fallo al guardar el equipo.\n";
                }
                break;
            }
            case 2: {
                int id;
                cout << "Ingrese ID de Equipo a leer: ";
                leerEntero(id);
                // Caso de Uso 2: Demostración matemática del Offset
                int indice = buscarIndicePorID(PATH_EQUIPOS, id, sizeof(Equipo));
                if (indice != -1) {
                    Equipo eq;
                    if (leerEquipoPorID(id, eq)) {
                        cout << "DEBUG: Registro en indice fisico (" << indice << ")\n";
                        cout << "Cálculo de Offset en Bytes: sizeof(Header) + (" << indice << " * " << sizeof(Equipo) << ")\n";
                        cout << "Posicion de lectura exacta: " << sizeof(ArchivoHeader) + (indice * sizeof(Equipo)) << " bytes.\n\n";
                        cout << "ID: " << eq.id << "\nNombre: " << eq.nombre << "\nCiudad: " << eq.ciudad << "\nEntrenador: " << eq.entrenador << "\nPuntos: " << eq.puntos << "\n";
                    }
                } else {
                    cout << "Equipo no encontrado o eliminado.\n";
                }
                break;
            }
            case 3: {
                char sub[100];
                cout << "Subcadena de busqueda: ";
                cin.getline(sub, 99);
                Equipo resultados[MAX_RESULTADOS];
                int cant = buscarEquiposPorNombre(sub, resultados, MAX_RESULTADOS);
                cout << "\nEquipos encontrados (" << cant << "):\n";
                for (int i = 0; i < cant; i++) {
                    cout << " - ID: " << resultados[i].id << " | Nombre: " << resultados[i].nombre << " | Ciudad: " << resultados[i].ciudad << "\n";
                }
                break;
            }
            case 4: {
                int id;
                cout << "ID del equipo a actualizar: ";
                leerEntero(id);
                Equipo eq;
                if (leerEquipoPorID(id, eq)) {
                    cout << "Nombre actual [" << eq.nombre << "]. Nuevo: ";
                    cin.getline(eq.nombre, 99);
                    cout << "Ciudad actual [" << eq.ciudad << "]. Nueva: ";
                    cin.getline(eq.ciudad, 99);
                    cout << "Entrenador actual [" << eq.entrenador << "]. Nuevo: ";
                    cin.getline(eq.entrenador, 99);
                    if (actualizarEquipo(eq)) {
                        cout << "Equipo modificado con exito.\n";
                    } else {
                        cout << "Error al reescribir equipo.\n";
                    }
                } else {
                    cout << "Equipo no encontrado.\n";
                }
                break;
            }
            case 5: {
                int id;
                cout << "ID del equipo a eliminar: ";
                leerEntero(id);
                if (confirmarAccion("Esta seguro de eliminar logicamente este equipo?")) {
                    if (eliminarEquipoLogico(id)) {
                        cout << "Equipo eliminado de forma logica con exito.\n";
                    } else {
                        cout << "No se pudo realizar la eliminacion.\n";
                    }
                }
                break;
            }
            case 6: {
                ArchivoHeader h = leerHeader(PATH_EQUIPOS);
                cout << "=== LISTADO DE EQUIPOS ACTIVOS (Header Registros Activos: " << h.registrosActivos << ") ===\n";
                ifstream arch(PATH_EQUIPOS, ios::binary);
                if (arch) {
                    arch.seekg(sizeof(ArchivoHeader), ios::beg);
                    Equipo temp;
                    for (int i = 0; i < h.cantidadRegistros; i++) {
                        arch.read(reinterpret_cast<char*>(&temp), sizeof(Equipo));
                        if (!temp.eliminado) {
                            cout << "ID: " << temp.id << " | " << temp.nombre << " (" << temp.ciudad << ") | DT: " << temp.entrenador << " | PTS: " << temp.puntos << "\n";
                        }
                    }
                    arch.close();
                }
                break;
            }
        }
        if (opcion != 0) {
            cout << "\nPresione Enter para continuar...";
            cin.get();
        }
    } while (opcion != 0);
}

void menuJugadores() {
    int opcion;
    do {
        system("cls");
        cout << "==========================================\n";
        cout << "          GESTION DE JUGADORES            \n";
        cout << "==========================================\n";
        cout << "1. Registrar Jugador\n";
        cout << "2. Buscar Jugador por ID\n";
        cout << "3. Listar Jugadores por Equipo\n";
        cout << "4. Actualizar Jugador\n";
        cout << "5. Eliminar Jugador (Logico)\n";
        cout << "0. Volver\n";
        cout << "==========================================\n";
        cout << "Seleccione: ";
        leerEntero(opcion);

        switch (opcion) {
            case 1: {
                Jugador jug = {};
                cout << "Ingrese ID de Equipo al que pertenece: ";
                leerEntero(jug.idEquipo);
                Equipo eq;
                if (!leerEquipoPorID(jug.idEquipo, eq)) {
                    cout << "ERROR: El equipo especificado no existe o fue eliminado.\n";
                    break;
                }
                cout << "Nombre completo: ";
                cin.getline(jug.nombre, 99);
                cout << "Cedula de identidad: ";
                cin.getline(jug.cedula, 19);
                cout << "Posicion: ";
                cin.getline(jug.posicion, 19);
                cout << "Edad: ";
                leerEntero(jug.edad);
                cout << "Numero de Dorsal: ";
                leerEntero(jug.numeroDorsal);

                if (guardarJugador(jug)) {
                    cout << "Jugador guardado con exito! ID Asignado: " << jug.id << " asociado a " << eq.nombre << "\n";
                } else {
                    cout << "Error al guardar el jugador.\n";
                }
                break;
            }
            case 2: {
                int id;
                cout << "ID del jugador a buscar: ";
                leerEntero(id);
                Jugador jug;
                if (leerJugadorPorID(id, jug)) {
                    Equipo eq;
                    char teamName[100];
                    if (leerEquipoPorID(jug.idEquipo, eq)) strcpy(teamName, eq.nombre);
                    else strcpy(teamName, "Desconocido (Roto)");

                    cout << "\nID: " << jug.id << "\nNombre: " << jug.nombre << "\nPosicion: " << jug.posicion << "\nEquipo: " << teamName << "\nGoles: " << jug.golesAnotados << "\n";
                } else {
                    cout << "Jugador no encontrado.\n";
                }
                break;
            }
            case 3: {
                int idEq;
                cout << "ID del equipo: ";
                leerEntero(idEq);
                Jugador res[MAX_RESULTADOS];
                int cant = listarJugadoresPorEquipo(idEq, res, MAX_RESULTADOS);
                cout << "\nJugadores encontrados (" << cant << "):\n";
                for (int i = 0; i < cant; i++) {
                    cout << " - ID: " << res[i].id << " | " << res[i].nombre << " | Posicion: " << res[i].posicion << " | Dorsal: " << res[i].numeroDorsal << " | Goles: " << res[i].golesAnotados << "\n";
                }
                break;
            }
            case 4: {
                int id;
                cout << "ID del jugador a editar: ";
                leerEntero(id);
                Jugador jug;
                if (leerJugadorPorID(id, jug)) {
                    cout << "Nombre [" << jug.nombre << "]. Nuevo: ";
                    cin.getline(jug.nombre, 99);
                    cout << "Posicion [" << jug.posicion << "]. Nueva: ";
                    cin.getline(jug.posicion, 19);
                    cout << "Edad [" << jug.edad << "]. Nueva: ";
                    leerEntero(jug.edad);
                    cout << "Dorsal [" << jug.numeroDorsal << "]. Nuevo: ";
                    leerEntero(jug.numeroDorsal);

                    if (actualizarJugador(jug)) {
                        cout << "Registro de jugador reescrito con exito.\n";
                    }
                } else {
                    cout << "No existe el jugador.\n";
                }
                break;
            }
            case 5: {
                int id;
                cout << "ID del jugador a eliminar: ";
                leerEntero(id);
                if (confirmarAccion("Seguro de eliminar logicamente a este jugador?")) {
                    if (eliminarJugadorLogico(id)) {
                        cout << "Jugador eliminado.\n";
                    }
                }
                break;
            }
        }
        if (opcion != 0) {
            cout << "\nPresione Enter para continuar...";
            cin.get();
        }
    } while (opcion != 0);
}

void menuPartidos() {
    int opcion;
    do {
        system("cls");
        cout << "==========================================\n";
        cout << "           GESTION DE PARTIDOS            \n";
        cout << "==========================================\n";
        cout << "1. Programar Partido (Crear)\n";
        cout << "2. Registrar Marcador y Detalle de Goles\n";
        cout << "3. Cancelar Partido Jugado (Reversión Completa)\n";
        cout << "4. Listar Partidos por Estado\n";
        cout << "0. Volver\n";
        cout << "==========================================\n";
        cout << "Seleccione: ";
        leerEntero(opcion);

        switch(opcion) {
            case 1: {
                Partido p = {};
                cout << "ID Equipo Local: ";
                leerEntero(p.idEquipoLocal);
                cout << "ID Equipo Visitante: ";
                leerEntero(p.idEquipoVisitante);

                Equipo el, ev;
                if (!leerEquipoPorID(p.idEquipoLocal, el) || !leerEquipoPorID(p.idEquipoVisitante, ev)) {
                    cout << "ERROR: Uno o ambos equipos no existen o estan eliminados.\n";
                    break;
                }

                cout << "Fecha (AAAA-MM-DD): ";
                cin.getline(p.fecha, 10);
                cout << "Descripcion: ";
                cin.getline(p.descripcion, 199);

                if (guardarPartido(p)) {
                    cout << "Partido programado con exito! ID Asignado: " << p.id << "\n";
                }
                break;
            }
            case 2: {
                int idPart;
                cout << "Ingrese ID del partido PROGRAMADO a jugar: ";
                leerEntero(idPart);

                Partido part;
                if (!leerPartidoPorID(idPart, part)) {
                    cout << "El partido no existe o está eliminado.\n";
                    break;
                }
                if (strcmp(part.estado, "PROGRAMADO") != 0) {
                    cout << "El partido ya fue jugado o cancelado anteriormente.\n";
                    break;
                }

                int golesL, golesV;
                cout << "Goles del Equipo Local: ";
                leerEntero(golesL);
                cout << "Goles del Equipo Visitante: ";
                leerEntero(golesV);

                int totalGoles = golesL + golesV;
                if (totalGoles > 22) {
                    cout << "ERROR: La cantidad total de goles supera el limite de 22.\n";
                    break;
                }

                Gol detalles[22];
                for (int i = 0; i < totalGoles; i++) {
                    cout << "\n--- REGISTRO DE GOL " << (i + 1) << " ---\n";
                    cout << "Minuto del gol (1-120): ";
                    leerEntero(detalles[i].minuto);
                    cout << "De que bando fue? (1 = LOCAL, 2 = VISITANTE): ";
                    int opBando;
                    leerEntero(opBando);
                    if (opBando == 1) strcpy(detalles[i].equipo, "LOCAL");
                    else strcpy(detalles[i].equipo, "VISITANTE");

                    cout << "ID del Jugador que anoto (0 si es desconocido / autogol): ";
                    leerEntero(detalles[i].idJugador);

                    if (detalles[i].idJugador > 0) {
                        Jugador tempJug;
                        if (!leerJugadorPorID(detalles[i].idJugador, tempJug)) {
                            cout << "Jugador no encontrado. Se guardará como desconocido.\n";
                            detalles[i].idJugador = 0;
                        }
                    }
                }

                if (registrarResultadoPartido(idPart, golesL, golesV, detalles, totalGoles)) {
                    cout << "\nResultado registrado con exito! Estadisticas y goles guardados de inmediato.\n";
                } else {
                    cout << "\nError al procesar la transaccion del partido.\n";
                }
                break;
            }
            case 3: {
                int idPart;
                cout << "Ingrese ID del partido JUGADO a cancelar: ";
                leerEntero(idPart);
                if (confirmarAccion("Esto revertira puntos, goles e historial. Seguro?")) {
                    if (cancelarPartidoJugado(idPart)) {
                        cout << "Partido cancelado y estadisticas revertidas con éxito.\n";
                    } else {
                        cout << "Fallo en la ejecucion de la reversion.\n";
                    }
                }
                break;
            }
            case 4: {
                cout << "Estado a listar (PROGRAMADO, JUGADO o CANCELADO): ";
                char est[15];
                cin.getline(est, 14);
                Partido partidos[MAX_RESULTADOS];
                int cant = listarPartidosPorEstado(est, partidos, MAX_RESULTADOS);
                cout << "\nPartidos en estado " << est << " (" << cant << "):\n";
                for (int i = 0; i < cant; i++) {
                    Equipo el, ev;
                    char nameL[100], nameV[100];
                    if (leerEquipoPorID(partidos[i].idEquipoLocal, el)) strcpy(nameL, el.nombre);
                    else strcpy(nameL, "Desconocido");
                    if (leerEquipoPorID(partidos[i].idEquipoVisitante, ev)) strcpy(nameV, ev.nombre);
                    else strcpy(nameV, "Desconocido");

                    cout << " - ID: " << partidos[i].id << " | " << nameL << " " << partidos[i].golesLocal << " vs " << partidos[i].golesVisitante << " " << nameV << " | Fecha: " << partidos[i].fecha << "\n";
                }
                break;
            }
        }
        if (opcion != 0) {
            cout << "\nPresione Enter para continuar...";
            cin.get();
        }
    } while (opcion != 0);
}

void menuReportes() {
    int opcion;
    do {
        system("cls");
        cout << "==========================================\n";
        cout << "                REPORTES                  \n";
        cout << "==========================================\n";
        cout << "1. Ver Tabla de Posiciones\n";
        cout << "2. Ver Tabla de Goleadores (Top 10)\n";
        cout << "3. Ver Ficha Tecnica de Partido\n";
        cout << "0. Volver\n";
        cout << "==========================================\n";
        cout << "Seleccione: ";
        leerEntero(opcion);

        switch(opcion) {
            case 1:
                system("cls");
                mostrarTablaPosiciones();
                break;
            case 2:
                system("cls");
                mostrarTablaGoleadores();
                break;
            case 3: {
                int id;
                cout << "Ingrese ID del partido para desplegar ficha: ";
                leerEntero(id);
                system("cls");
                mostrarFichaTecnicaPartido(id);
                break;
            }
        }
        if (opcion != 0) {
            cout << "\nPresione Enter para continuar...";
            cin.get();
        }
    } while (opcion != 0);
}

void menuMantenimiento() {
    int opcion;
    do {
        system("cls");
        cout << "==========================================\n";
        cout << "             MANTENIMIENTO                \n";
        cout << "==========================================\n";
        cout << "1. Verificar Integridad Referencial\n";
        cout << "2. Crear Backup de Datos (.bin)\n";
        cout << "0. Volver\n";
        cout << "==========================================\n";
        cout << "Seleccione: ";
        leerEntero(opcion);

        switch(opcion) {
            case 1: {
                system("cls");
                cout << "Ejecutando verificacion de sanidad referencial...\n\n";
                int eq = 0, jug = 0, part = 0, rotas = 0;
                verificarIntegridadReferencial(eq, jug, part, rotas);

                cout << "\n";
                cout << "=========================================================\n";
                cout << "           REPORTE DE INTEGRIDAD REFERENCIAL             \n";
                cout << "=========================================================\n";
                cout << "  Equipos verificados   : " << eq << "\n";
                cout << "  Jugadores verificados : " << jug << "\n";
                cout << "  Partidos verificados  : " << part << "\n\n";
                cout << "  Referencias rotas encontradas: " << rotas << "\n";
                if (rotas == 0) {
                    cout << "  Estado: [SISTEMA INTEGRO]\n";
                } else {
                    cout << "  Estado: [ADVERTENCIA - REFERENCIAS ROTAS ACTIVAS]\n";
                }
                cout << "=========================================================\n";
                break;
            }
            case 2: {
                char backupPath[150];
                if (crearBackup(backupPath)) {
                    cout << "Backup generado con exito.\n";
                    cout << "Ruta de carpeta creada: " << backupPath << "\n";
                } else {
                    cout << "ERROR al generar la copia de respaldo.\n";
                }
                break;
            }
        }
        if (opcion != 0) {
            cout << "\nPresione Enter para continuar...";
            cin.get();
        }
    } while (opcion != 0);
}

// ==========================================
// FUNCIÓN PRINCIPAL DE ENTRADA
// ==========================================

int main() {
    // Forzamos soporte para acentuación local en consola
    setlocale(LC_ALL, "spanish");

    if (!inicializarSistemaArchivos()) {
        cout << "ERROR CRITICO: No se pudieron mapear e inicializar los archivos binarios.\n";
        return 1;
    }

    int opcion;
    do {
        system("cls");
        Torneo t;
        leerTorneo(t);

        cout << "==========================================\n";
        cout << "      SISTEMA DE GESTION DE TORNEOS       \n";
        cout << "      " << t.nombre << "                  \n";
        cout << "==========================================\n";
        cout << "1. Gestion de Equipos\n";
        cout << "2. Gestion de Jugadores\n";
        cout << "3. Gestion de Partidos\n";
        cout << "4. Ver Tabla de Posiciones\n";
        cout << "5. Reportes Especiales\n";
        cout << "6. Mantenimiento del Sistema\n";
        cout << "0. Salir\n";
        cout << "==========================================\n";
        cout << "Seleccione una opcion: ";
        leerEntero(opcion);

        switch(opcion) {
            case 1: menuEquipos(); break;
            case 2: menuJugadores(); break;
            case 3: menuPartidos(); break;
            case 4: 
                system("cls");
                mostrarTablaPosiciones();
                cout << "\nPresione Enter para continuar...";
                cin.get();
                break;
            case 5: menuReportes(); break;
            case 6: menuMantenimiento(); break;
            case 0:
                cout << "Gracias por usar el software de gestion. Cerrando archivos binarios...\n";
                break;
            default:
                cout << "Opcion incorrecta.\n";
                break;
        }
    } while (opcion != 0);

    return 0;
}