#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/complex.h>
#include <nanobind/stl/vector.h>
#include <vector>
#include "radarData.h"
#include "stdlib.h"
#include "radarhandler.h"
#include "ahm_common.h"


namespace nb = nanobind;
using std::string;

NB_MODULE(AHMAPIpy, m) {

    m.doc() = "Radar module";

    // Binds rdrElabTypes_t
    nb::enum_<rdrElabTypes_t>(m, "rdrElabTypes_t")
        .value("RT_UNKNOWN", RT_UNKNOWN)
        .value("RT_RAW", RT_RAW)
        .value("RT_MTI", RT_MTI)
        .export_values();

    // Binds profile_t
    nb::enum_<profile_t>(m, "profile_t")
        .value("PROF_SR_MID_RES", PROF_SR_MID_RES)
        .value("PROF_SR_HIGH_RES", PROF_SR_HIGH_RES)
        .value("PROF_LR_MID_RES", PROF_LR_MID_RES)
        .value("PROF_LR_HIGH_RE", PROF_LR_HIGH_RE)
        .export_values();

    // Binds the struct canvas2d_param_t
    nb::class_<canvas2d_param_t>(m, "canvas2d_param_t")
        .def(nb::init<>())
        .def_rw("rhoMin",  &canvas2d_param_t::rhoMin)
        .def_rw("rhoMax",  &canvas2d_param_t::rhoMax)
        .def_rw("rhoStep", &canvas2d_param_t::rhoStep)
        .def_rw("phiMin",  &canvas2d_param_t::phiMin)
        .def_rw("phiMax",  &canvas2d_param_t::phiMax)
        .def_rw("phiStep", &canvas2d_param_t::phiStep)
        .def_rw("algo",    &canvas2d_param_t::algo);

    nb::class_<ant_t>(m, "ant_t")
        .def(nb::init<>())
        .def_rw("x",  &ant_t::x)
        .def_rw("y",  &ant_t::y)
        .def_rw("delay_s", &ant_t::delay_s)
        .def_rw("ampl",  &ant_t::ampl);

    nb::class_<txAntDesc_t>(m, "txAntDesc_t")
        .def(nb::init<>())
        // Proprietà in sola lettura che ritorna una tupla Python di oggetti ant_t
        .def_prop_ro("ant", [](const txAntDesc_t &self) {
            return nb::make_tuple(
                nb::cast(self.ant[0], nb::rv_policy::copy),
                nb::cast(self.ant[1], nb::rv_policy::copy),
                nb::cast(self.ant[2], nb::rv_policy::copy),
                nb::cast(self.ant[3], nb::rv_policy::copy)
                );
        });

    nb::class_<rxAntDesc_t>(m, "rxAntDesc_t")
        .def(nb::init<>())
        .def(nb::init<>())
        .def_prop_ro("ant", [](const rxAntDesc_t &self) {
            return nb::make_tuple(
                nb::cast(self.ant[0], nb::rv_policy::copy),
                nb::cast(self.ant[1], nb::rv_policy::copy),
                nb::cast(self.ant[2], nb::rv_policy::copy),
                nb::cast(self.ant[3], nb::rv_policy::copy)
                );
        });


    // Binds the recAlgo_t
    nb::enum_<recAlgo_t>(m, "recAlgo_t")
        .value("RA_DAS", RA_DAS)
        .value("RA_DMAS", RA_DMAS)
        .export_values();

    // Binds the struct canvas3d_param_t
    nb::class_<canvas3d_param_t>(m, "canvas3d_param_t")
        .def(nb::init<>())
        .def_rw("rhoMin",   &canvas3d_param_t::rhoMin)
        .def_rw("rhoMax",   &canvas3d_param_t::rhoMax)
        .def_rw("rhoStep",  &canvas3d_param_t::rhoStep)
        .def_rw("elMin",    &canvas3d_param_t::elMin)
        .def_rw("elMax",    &canvas3d_param_t::elMax)
        .def_rw("elStep",   &canvas3d_param_t::elStep)
        .def_rw("azimMin",  &canvas3d_param_t::azimMin)
        .def_rw("azimMax",  &canvas3d_param_t::azimMax)
        .def_rw("azimStep", &canvas3d_param_t::azimStep)
        .def_rw("algo",     &canvas3d_param_t::algo);

    // Binds the struct, with this instruction
    nb::class_<scanSequence_t>(m, "ScanSequence")
        .def(nb::init<>())
        .def_rw("numAnt", &scanSequence_t::numAnt)

        // Exposes the txI array (Getter and Setter)
        .def_prop_rw("txI",
                  // GETTER
                  [](const scanSequence_t &self) {
                      return std::vector<uint8_t>(self.txI, self.txI + 16);
                  },
                  // SETTER: Invoked when writing to the property in Python
                  [](scanSequence_t &self, const std::vector<uint8_t> &val) {
                      size_t size = std::min(val.size(), (size_t)16);
                      for (size_t i = 0; i < size; ++i) {
                          self.txI[i] = val[i];
                      }
                  }
        )

        // Exposes the rxI array (Getter and Setter)
        .def_prop_rw("rxI",
                  // GETTER: Invoked when reading the property in Python
                  [](const scanSequence_t &self) {
                      return std::vector<uint8_t>(self.rxI, self.rxI + 16);
                  },
                  // SETTER: Invoked when writing to the property in Python
                  [](scanSequence_t &self, const std::vector<uint8_t> &val) {
                      size_t size = std::min(val.size(), (size_t)16);
                      for (size_t i = 0; i < size; ++i) {
                          self.rxI[i] = val[i];
                      }
                  }
        );

    // Binds the class
    nb::class_<RadarHandler>(m, "RadarHandler")

        // Binds the constractors
        .def(nb::init<string, string>(), nb::arg("serial_comunication"), nb::arg("COM"))
        .def(nb::init<string, string, int>(), nb::arg("serial_comunication"), nb::arg("COM"), nb::arg("BR"))

        // Binds the methods
        .def("rdr_start", &RadarHandler::rdr_start)
        .def("rdr_stop", &RadarHandler::rdr_stop)
        .def("rdr_set_frame_rate", &RadarHandler::rdr_set_frame_rate)
        .def("rdr_get_frame_rate", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_frame_rate(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_tx_power", &RadarHandler::rdr_set_tx_power)
        .def("rdr_get_tx_power", [](RadarHandler &self) {
            uint8_t y=0;
            int ret = self.rdr_get_tx_power(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(y,ret);
        })
        .def("rdr_set_min_Xrange", &RadarHandler::rdr_set_min_Xrange)
        .def("rdr_get_min_Xrange", [](RadarHandler &self) {
            float y=0;
            int ret = self.rdr_get_min_Xrange(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_max_Xrange", &RadarHandler::rdr_set_max_Xrange)
        .def("rdr_get_max_Xrange", [](RadarHandler &self) {
            float y=0;
            int ret = self.rdr_get_max_Xrange(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_offset", &RadarHandler::rdr_set_offset)
        .def("rdr_get_offset", [](RadarHandler &self) {
            float y=0;
            int ret = self.rdr_get_offset(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_iterations", &RadarHandler::rdr_set_iterations)
        .def("rdr_get_iterations", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_iterations(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_declutter_length", &RadarHandler::rdr_set_declutter_length)
        .def("rdr_get_declutter_length", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_declutter_length(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_elabtype", &RadarHandler::rdr_set_elabtype)
        .def("rdr_get_elabtype", [](RadarHandler &self) {
            rdrElabTypes_t y = RT_UNKNOWN;
            int ret = self.rdr_get_elabtype(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_optproc", &RadarHandler::rdr_set_optproc)
        .def("rdr_get_optproc", [](RadarHandler &self) {
            uint8_t x=0;
            uint8_t y=0;
            int ret = self.rdr_get_optproc(&x, &y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(x, y);
        })
        .def("rdr_set_sequence", &RadarHandler::rdr_set_sequence)
        .def("rdr_get_sequence", [](RadarHandler &self) {
            uint8_t x[16];
            uint8_t y[16];
            uint8_t z;
            int ret = self.rdr_get_sequence(x, y, &z);
            printf("Errore: %d\n",ret);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(x, y, z);
        })
        .def("rdr_set_freq_carrier", &RadarHandler::rdr_set_freq_carrier)
        .def("rdr_get_freq_carrier", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_freq_carrier(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_bw", &RadarHandler::rdr_set_bw)
        .def("rdr_get_bw", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_bw(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_bwmode", &RadarHandler::rdr_set_bwmode)
        .def("rdr_get_bwmode", [](RadarHandler &self) {
            uint8_t y=0;
            int ret = self.rdr_get_bwmode(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_code", &RadarHandler::rdr_set_code)
        .def("rdr_get_code", [](RadarHandler &self) {
            int8_t x;
            int y;
            int ret = self.rdr_get_code(&x, &y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(x, y);
        })
        .def("rdr_set_gain", &RadarHandler::rdr_set_gain)
        .def("rdr_get_gain", [](RadarHandler &self) {
            uint16_t x;
            uint16_t y;
            int ret = self.rdr_get_gain(&x, &y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(x, y);
        })
        .def("rdr_get_fSampling", [](RadarHandler &self) {
            uint8_t y=0;
            int ret = self.rdr_get_bwmode(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_get_arrayDescr",  [](RadarHandler &self) {
            txAntDesc_t x;
            rxAntDesc_t y;
            int ret = self.rdr_get_arrayDescr(&x, &y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(x, y);
        })
        .def("rdr_set_canvas_2d", nb::overload_cast<canvas3d_param_t>(&RadarHandler::rdr_set_canvas))
        .def("rdr_get_canvas_2d", [](RadarHandler &self) {
            canvas2d_param_t y;
            int ret = self.rdr_get_canvas(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_canvas_3d", nb::overload_cast<canvas3d_param_t>(&RadarHandler::rdr_set_canvas))
        .def("rdr_get_canvas_3d", [](RadarHandler &self) {
            canvas3d_param_t y;
            int ret = self.rdr_get_canvas(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_fmt", &RadarHandler::rdr_set_fmt)
        .def("rdr_get_fmt", [](RadarHandler &self) {
            uint8_t y=0;
            int ret = self.rdr_get_fmt(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_baudrate", &RadarHandler::rdr_set_baudrate)
        .def("rdr_set_multistream", &RadarHandler::rdr_set_multistream)
        .def("rdr_get_multistream", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_multistream(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_get_bw", &RadarHandler::rdr_get_bw)
        .def("rdr_conv_mask2sequence", [](RadarHandler &self, uint8_t txMask, uint8_t rxMask,  int num) {
            scanSequence_t seq;
            int state = self.rdr_conv_mask2sequence(&txMask, &rxMask, &seq, num);

            if (state != 0) {
                throw std::runtime_error("Communication problem");
            }

            return seq;
        }, nb::arg("txMask"), nb::arg("rxMask"), nb::arg("num"))
        .def("rdr_get_image", [](RadarHandler &self) {

            radarData<float> rd;
            int state = self.rdr_get_image(&rd);

            if (state != 0) {
                throw std::runtime_error("Communication problem");
            }

            size_t rows = rd.numRows();
            size_t cols = rd.numClmns();

            std::vector<float> data(rows * cols);
            std::vector<float> v(cols);

            for (size_t r = 0; r < rows; r++) {
                rd.getRaw(r, v.data()); // Lettura di una riga
                float * row_ptr = data.data() + (r * cols);
                for (size_t c = 0; c < cols; c++) {
                    row_ptr[c] = *reinterpret_cast<float*>(&v[c]);
                }
            }

            // Funzione cast necessaria altrimenti crea il numby array ma poi lo distrugge
            return nb::ndarray<nb::numpy, float, nb::ndim<2>>(
                       data.data(), // Puntatore ai dati
                       { rows, cols }       // Shape
                       ).cast();
        })

        .def("rdr_rx_multichannel", [](RadarHandler &self) {

            radarData<cplx_float_t> rd;
            int state = self.rdr_rx_multichannel(&rd);

            if (state != 0) {
                throw std::runtime_error("Communication problem");
            }

            size_t rows = rd.numRows();
            size_t cols = rd.numClmns();

            // Viene memorizzato std::complex<float> per compatibilità con numby complex64, ndarray inoltro estrae la matrice andando
            // valutare la memoria contigua che gli viene indicata come parametro shape per questo viene creato il vettore di appoggio
            std::vector<std::complex<float>> data(rows * cols);
            std::vector<cplx_float_t> v(cols);

            for (size_t r = 0; r < rows; r++) {
                rd.getRaw(r, v.data()); // Lettura di una riga
                std::complex<float>* row_ptr = data.data() + (r * cols);
                for (size_t c = 0; c < cols; c++) {
                    row_ptr[c] = *reinterpret_cast<std::complex<float>*>(&v[c]);
                }
            }

            // Funzione cast necessaria altrimenti crea il numby array ma poi lo distrugge
            return nb::ndarray<nb::numpy, std::complex<float>, nb::ndim<2>>(
                       data.data(), // Puntatore ai dati
                       { rows, cols }       // Shape
                       ).cast();
        })
        // new
        .def("rdr_set_profile", &RadarHandler::rdr_set_profile)
        .def("rdr_set_staticObjRemoval", &RadarHandler::rdr_set_staticObjRemoval)
        .def("rdr_get_staticObjRemoval", [](RadarHandler &self){
            uint8_t y=0;
            int ret = self.rdr_get_staticObjRemoval(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
                }
            return y;
        })
        .def("rdr_set_staticObjMapUpdateTime", &RadarHandler::rdr_set_staticObjMapUpdateTime)
        .def("rdr_get_staticObjMapUpdateTime", [](RadarHandler &self){
            float y=0;
            int ret = self.rdr_get_staticObjMapUpdateTime(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_rxGain", &RadarHandler::rdr_set_rxGain)
        .def("rdr_get_rxGain", [](RadarHandler &self){
            uint8_t y=0;
            int ret = self.rdr_get_rxGain(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        })
        .def("rdr_set_embeddedImgAlgo", &RadarHandler::rdr_set_embeddedImgAlgo)
        .def("rdr_get_embeddedImgAlgo", [](RadarHandler &self){
            uint8_t y=0;
            int ret = self.rdr_get_embeddedImgAlgo(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        });
}

