#include "../functions.hpp"
#include "../simulator.hpp"
#include <random>
#include <chrono>

BEGIN_ALLOC_FUNCTION(FirstFit) {
    // Variables de control
    int largoRuta;
    std::vector<char> ordenBandas;
    std::map<char, std::vector<bool>> estadoSlots;
    int requerido;
    int total;
    int indice;

    // Métricas
    totalBitRate[mapaBitRate[REQ_BITRATE]]++;
    
    for (size_t r = 0; r < NUMBER_OF_ROUTES; r++){ // RBMLSA: -> R (Por cada ruta).

        // Obtenemos el largo de la ruta y el orden de las bandas.
        largoRuta = 0;
        for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++){ 
            largoRuta += LINK_IN_ROUTE(r, l)->getLength();
        }
        ordenBandas = ORDEN(largoRuta, escenario);

        for (size_t m = 0; m < NUMBER_OF_MODULATIONS; m++){ // RBMLSA: -> ML (Por cada modulación)
            for (size_t b = 0; b < NUMBER_OF_BANDS(r, 0); b++){ // RBMLSA: -> B (Por cada banda).

                // Verificamos si la modulación es factible.
                if (REQ_REACH_BDM(m, REQ_POS_BANDS(m)[ordenBandas[b]]) < largoRuta) continue;

                // Slots requeridos por la modulación y bit rate actual
                requerido = REQ_SLOTS_BDM(m, REQ_POS_BANDS(m)[ordenBandas[b]]);

                // Obtenemos el vector representativo de los slots ocupados en la ruta (S).
                if (estadoSlots.find(ordenBandas[b]) == estadoSlots.end()) {
                    // Inicializamos el vector de slots para la banda actual si no existe en el mapa
                    estadoSlots[ordenBandas[b]] = std::vector<bool>(LINK_IN_ROUTE(r, 0)->getSlots(ordenBandas[b]), false);

                    // Actualizamos el estado de cada slot en función de cada enlace en la ruta
                    for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++) {
                        for (size_t s = 0; s < LINK_IN_ROUTE(r, l)->getSlots(ordenBandas[b]); s++) { // Por cada slot
                            estadoSlots[ordenBandas[b]][s] = estadoSlots[ordenBandas[b]][s] | LINK_IN_ROUTE(r, l)->getSlot(s, ordenBandas[b]);
                        }
                    }
                }

                // Buscamos slots consecutivos libres para la asignación.
                total = 0;
                indice = 0;
                for (size_t s = 0; s < estadoSlots[ordenBandas[b]].size(); s++){ // RBMLSA: ->SA (Por cada slot).
                    if (estadoSlots[ordenBandas[b]][s] == false){
                        total++;
                    }
                    else {
                        total = 0;
                        indice = s + 1;
                    }
                    if (total == requerido){
                        // Asignar slots en caso de encontrar lighpath factible.
                        for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++){
                            ALLOC_SLOTS_BDM(LINK_IN_ROUTE_ID(r, l), ordenBandas[b], indice, requerido);
                        }
                        return ALLOCATED;
                    }
                }
            }
        }       
    }
    bloqueadosBitRate[mapaBitRate[REQ_BITRATE]]++;
    return NOT_ALLOCATED;
}
END_ALLOC_FUNCTION

BEGIN_ALLOC_FUNCTION(BestFit) {
    // Variables de control
    int largoRuta;
    std::vector<char> ordenBandas;
    std::map<char, std::vector<bool>> estadoSlots;
    int requerido;
    int total;
    int indice;
    int mejorTotal;
    int mejorIndice;

    // Métricas
    totalBitRate[mapaBitRate[REQ_BITRATE]]++;
    
    for (size_t r = 0; r < NUMBER_OF_ROUTES; r++){ // RBMLSA: -> R (Por cada ruta).

        // Obtenemos el largo de la ruta y el orden de las bandas.
        largoRuta = 0;
        for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++){ 
            largoRuta += LINK_IN_ROUTE(r, l)->getLength();
        }
        ordenBandas = ORDEN(largoRuta, escenario);

        for (size_t m = 0; m < NUMBER_OF_MODULATIONS; m++){ // RBMLSA: -> ML (Por cada modulación)
            for (size_t b = 0; b < NUMBER_OF_BANDS(r, 0); b++){ // RBMLSA: -> B (Por cada banda).

                // Verificamos si la modulación es factible.
                if (REQ_REACH_BDM(m, REQ_POS_BANDS(m)[ordenBandas[b]]) < largoRuta) continue;

                // Slots requeridos por la modulación y bit rate actual
                requerido = REQ_SLOTS_BDM(m, REQ_POS_BANDS(m)[ordenBandas[b]]);

                // Obtenemos el vector representativo de los slots ocupados en la ruta (S).
                if (estadoSlots.find(ordenBandas[b]) == estadoSlots.end()) {
                    // Inicializamos el vector de slots para la banda actual si no existe en el mapa
                    estadoSlots[ordenBandas[b]] = std::vector<bool>(LINK_IN_ROUTE(r, 0)->getSlots(ordenBandas[b]), false);

                    // Actualizamos el estado de cada slot en función de cada enlace en la ruta
                    for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++) {
                        for (size_t s = 0; s < LINK_IN_ROUTE(r, l)->getSlots(ordenBandas[b]); s++) { // Por cada slot
                            estadoSlots[ordenBandas[b]][s] = estadoSlots[ordenBandas[b]][s] | LINK_IN_ROUTE(r, l)->getSlot(s, ordenBandas[b]);
                        }
                    }
                }

                // Buscamos slots consecutivos libres para la asignación.
                total = 0;
                indice = 0;
                mejorTotal = std::numeric_limits<int>::max();
                mejorIndice = -1;
                for (size_t s = 0; s < estadoSlots[ordenBandas[b]].size(); s++){ // RBMLSA: ->SA (Por cada slot).
                    if (estadoSlots[ordenBandas[b]][s] == false){
                        total++;
                    }
                    else {
                        if (total == requerido){
                            // Asignar slots en caso de encontrar lighpath exacto.
                            for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++){
                                ALLOC_SLOTS_BDM(LINK_IN_ROUTE_ID(r, l), ordenBandas[b], indice, requerido);
                            }
                            return ALLOCATED;
                        }
                        if (total >= requerido && total < mejorTotal){
                            mejorTotal = total;
                            mejorIndice = indice;
                        }
                        total = 0;
                        indice = s + 1;
                    }
                }
                // Asignamos al mejor bloque de slots encontrado.
                if (total >= requerido && total < mejorTotal) {
                    mejorTotal = total;
                    mejorIndice = indice;
                }
                if (mejorIndice != -1){
                    for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++){
                        ALLOC_SLOTS_BDM(LINK_IN_ROUTE_ID(r, l), ordenBandas[b], mejorIndice, requerido);
                    }
                    return ALLOCATED;
                }
            }
        }       
    }
    bloqueadosBitRate[mapaBitRate[REQ_BITRATE]]++;
    return NOT_ALLOCATED;
}
END_ALLOC_FUNCTION

BEGIN_UNALLOC_CALLBACK_FUNCTION{

}
END_UNALLOC_CALLBACK_FUNCTION

int main(int argc, char* argv[]) {

    // Default
    pesoBitRate[0] = 1.0 * 0.5;
    pesoBitRate[1] = 1.5 * 0.5;
    pesoBitRate[2] = 3.0 * 0.5;
    pesoBitRate[3] = 2.25 * 0.5;

    // Abrir archivo de salida
    file.open("./results/test.csv", std::ios_base::app);

    // Setear variables de BBP
    for (size_t i = 0; i < 4; i++) {
        totalBitRate[i] = 0.0;
        bloqueadosBitRate[i] = 0.0;
    }

    // Setear conexiones por banda
    conexionesPorBanda['C'] = 0;
    conexionesPorBanda['L'] = 0;
    conexionesPorBanda['E'] = 0;
    conexionesPorBanda['S'] = 0;


    // Crear semillas
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    // int seedArrive = rng();
    // int seedBitRate = rng();
    // int seedDst = rng();
    // int seedDeparture = rng();
    // int seedBitRate = rng();

    int lambda = 1500;

    Simulator sim =
        Simulator(std::string("./testfiles/NSFNet.json"),
            std::string("./testfiles/routes.json"),
            std::string("./testfiles/bitrates.json"),
            BDM);

    USE_ALLOC_FUNCTION(FirstFit, sim);
    USE_UNALLOC_FUNCTION_BDM(sim);
    sim.setGoalConnections(nConexiones);
    sim.setLambda(lambda);
    sim.setMu(1);

    // Set Semillas
    // sim.setSeedArrive(seedArrive);
    // sim.setSeedBitRate(seedBitRate);
    // sim.setSeedDst(seedArrive);
    // sim.setSeedDeparture(seedArrive);
    // sim.setSeedDeparture(seedDeparture);

    sim.init();
    sim.run();

    // Exportar resultados
    file << lambda << "\t" << sim.getBlockingProbability() << "\t" << sim.wilsonCI() << "\t" << sim.wilsonCI() << "\\\\  " 
            << BBP(totalBitRate, bloqueadosBitRate, pesoBitRate) 
            << std::endl;

    // Asignaciones por banda
    file    << "C: ," << conexionesPorBanda['C'] << ")\n"
            << "L: ," << conexionesPorBanda['L'] << ")\n"
            << "E: ," << conexionesPorBanda['E']  << ")\n"
            << "S: ," << conexionesPorBanda['S']  << ")"
            << std::endl;
    file << std::endl;
    file.close();
    
    return 0;
}