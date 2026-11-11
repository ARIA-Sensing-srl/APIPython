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
    using namespace nanobind::literals;
    m.doc() = "Radar module";

    // Binds rdrElabTypes_t
    nb::enum_<rdrElabTypes_t>(m, "RdrElabTypes", "Represents the current elaboration level")
        .value("RT_UNKNOWN", RT_UNKNOWN)
        .value("RT_RAW", RT_RAW, "Raw data")
        .value("RT_MTI", RT_MTI, "MTI. Output of the declutter algorithm.")
        .export_values();

    // Binds profile_t
    nb::enum_<profile_t>(m, "profile", "Represents range and resolution profiles")
        .value("PROF_SR_MID_RES", PROF_SR_MID_RES, "Short range, mid resolution.")
        .value("PROF_SR_HIGH_RES", PROF_SR_HIGH_RES, "Short range, high resolution.")
        .value("PROF_LR_MID_RES", PROF_LR_MID_RES, "Long range, mid resolution.")
        .value("PROF_LR_HIGH_RE", PROF_LR_HIGH_RE, "Long range, high resolution.")
        .export_values();

    // Binds the struct canvas2d_param_t
    nb::class_<canvas2d_param_t>(m, "canvas2d_param", "Configuration parameters for a 2D canvas")
        .def(nb::init<>())
        .def_rw("rhoMin",  &canvas2d_param_t::rhoMin, "(float) downrange minimum distance expressed in meters.")
        .def_rw("rhoMax",  &canvas2d_param_t::rhoMax, "(float) downrange maximum distance expressed in meters.")
        .def_rw("rhoStep", &canvas2d_param_t::rhoStep, "(float) downrange stride in meters.")
        .def_rw("phiMin",  &canvas2d_param_t::phiMin, "(float) azimuth minimum range in degrees.")
        .def_rw("phiMax",  &canvas2d_param_t::phiMax, "(float) azimuth maximum range in degrees.")
        .def_rw("phiStep", &canvas2d_param_t::phiStep, "(float) azimuth stride.")
        .def_rw("algo",    &canvas2d_param_t::algo, "(enumerated recAlgo) selected reconstruction algorithm, 0: DAS with apodization, 1: DMAS with apodization.");

    nb::class_<ant_t>(m, "ant", "Represents antenna properties")
        .def(nb::init<>())
        .def_rw("x",  &ant_t::x, "(float) antenna’s X coordinates in meters.")
        .def_rw("y",  &ant_t::y, "(float) antenna’s Y coordinates in meters.")
        .def_rw("delay_s", &ant_t::delay_s,  "(float) propagation delay correction factor in seconds")
        .def_rw("ampl",  &ant_t::ampl, "(float) antenna’s amplitude correction factor (polarity) . If this value is set to 0, there’s no antenna connected to the channel.");

    nb::class_<txAntDesc_t>(m, "txAntDesc", "Contains information about transmit antennas")
        .def(nb::init<>())
        .def_prop_ro("ant", [](const txAntDesc_t &self) {
            nb::list l;
            for (size_t i = 0; i < 4; i++) {
                // nb::rv_policy::copy serve per creare una copia completa dell'oggetto C++ in memoria e fornire la proprietà di questa copia a Python
                l.append(nb::cast(self.ant[i], nb::rv_policy::copy));
            }
            return l;
        },"List of 4 transmit antenna properties (ant)");

    nb::class_<rxAntDesc_t>(m, "rxAntDesc", "Contains information about receive antennas")
        .def(nb::init<>())
        .def(nb::init<>())
        .def_prop_ro("ant", [](const rxAntDesc_t &self) {
            nb::list l;
            for (size_t i = 0; i < 4; i++) {
                // nb::rv_policy::copy serve per creare una copia completa dell'oggetto C++ in memoria e fornire la proprietà di questa copia a Python
                l.append(nb::cast(self.ant[i], nb::rv_policy::copy));
            }
            return l;
        },"List of 4 receive antenna properties (ant)");


    // Binds the recAlgo_t
    nb::enum_<recAlgo_t>(m, "recAlgo", "Contains information about the algorithm used internally")
        .value("RA_DAS", RA_DAS)
        .value("RA_DMAS", RA_DMAS)
        .export_values();
    /*
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
    */

    // Binds the struct, with this instruction
    nb::class_<scanSequence_t>(m, "ScanSequence", "Contains information about active transmit and receiver channels")
        .def(nb::init<>())
        .def_rw("numAnt", &scanSequence_t::numAnt)

        // Exposes the txI array (Getter and Setter)
        .def_prop_rw("txI",
                  // GETTER
                  [](const scanSequence_t &self) {
                      return std::vector<uint8_t>(self.txI, self.txI + 8);
                  },
                  // SETTER: Invoked when writing to the property in Python
                  [](scanSequence_t &self, const std::vector<uint8_t> &val) {
                      size_t size = std::min(val.size(), (size_t)8);
                      for (size_t i = 0; i < size; ++i) {
                          self.txI[i] = val[i];
                      }
                  }
        , "List of active transmit channels (int)")
        // Exposes the rxI array (Getter and Setter)
        .def_prop_rw("rxI",
                  // GETTER: Invoked when reading the property in Python
                  [](const scanSequence_t &self) {
                      return std::vector<uint8_t>(self.rxI, self.rxI + 8);
                  },
                  // SETTER: Invoked when writing to the property in Python
                  [](scanSequence_t &self, const std::vector<uint8_t> &val) {
                      size_t size = std::min(val.size(), (size_t)8);
                      for (size_t i = 0; i < size; ++i) {
                          self.rxI[i] = val[i];
                      }
                  }
        , "List of active receive channels (int)");

    nb::class_<RdrFWVer_t>(m, "RdrFWVer", "Contains information about the hardware and command set")
        .def(nb::init<>())
        .def_rw("customerID", &RdrFWVer_t::customerID)
        .def_rw("hwcode", &RdrFWVer_t::hwcode)
        .def_rw("IDver", &RdrFWVer_t::IDver)
        .def_rw("featureSet", &RdrFWVer_t::featureSet)
        .def_rw("IDsubVer", &RdrFWVer_t::IDsubVer);


    // Binds the class
    nb::class_<RadarHandler>(m, "RadarHandler", "Rapresent radar entity")

        // Binds the constractors
        .def(nb::init<string, string>(), nb::arg("serial_comunication"), nb::arg("COM"))
        .def(nb::init<string, string, int>(), nb::arg("serial_comunication"), nb::arg("COM"), nb::arg("BR"))

        // Binds the methods
        .def("rdr_start", &RadarHandler::rdr_start, "Start the radar")
        .def("rdr_stop", &RadarHandler::rdr_stop, "This function stops the device operations and sets the device in idle state.")

        .def("rdr_set_fps", &RadarHandler::rdr_set_fps, "fps"_a, R"doc(
        Set the frame per seconds of the radar.

        Args:
            fps (int): Desired frame rate.
        )doc")
        .def("rdr_get_fps", [](RadarHandler &self) {
            uint16_t y = 0;
            int ret = self.rdr_get_fps(&y);
            if (ret) {
                throw std::runtime_error("Communication problem with radar device.");
            }
            return y;
        }, R"doc(
        Get the current frame per seconds of the radar.

        Returns:
            int: The current device frame rate.
        )doc")

        .def("rdr_set_tx_power", &RadarHandler::rdr_set_tx_power, "tx_power"_a, R"doc(
        Set the transmit power of the antennas.

        Args:
            tx_power (int): Desired transmit power.
        )doc")
        .def("rdr_get_tx_power", [](RadarHandler &self) {
            uint8_t y=0;
            int ret = self.rdr_get_tx_power(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Get the transmit power of the antennas.

        Returns:
            int: The current device transmit power.
        )doc")

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

        .def("rdr_set_offset", &RadarHandler::rdr_set_offset, "offset"_a, R"doc(
        Get the global offset, expressed in meters, to align range configuration to ground true distances. This
        parameter is used for compensating internal global delays. It’s advisable to use the provided value
        (scripts or FW’s default)

        Args:
            offset (float): Desired global offset.
        )doc")
        .def("rdr_get_offset", [](RadarHandler &self) {
            float y=0;
            int ret = self.rdr_get_offset(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        },R"doc(
        Set the global offset, expressed in meters, to align range configuration to ground true distances. This
        parameter is used for compensating internal global delays. It’s advisable to use the provided value
        (scripts or FW’s default)

        Returns:
            float: The current device global offset.
        )doc")

        .def("rdr_set_iterations", &RadarHandler::rdr_set_iterations, "iterations"_a, R"doc(
        Set the number of iterations used for getting a single stream (tx-rx antenna pair).

        Args:
            iterations (int): Desired number of iterations.
        )doc")
        .def("rdr_get_iterations", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_iterations(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Get the number of iterations used for getting a single stream (tx-rx antenna pair).

        Returns:
            int: Number of iterations.
        )doc")

        .def("rdr_set_declutter_length", &RadarHandler::rdr_set_declutter_length,"declutter"_a, R"doc(
        Control the time constant used by the static object removal algorithm (clutter removal algorithm), by
        changing the equivalent number of acquisitions used to compute the static object map.

        Args:
            declutter (int): Number of frames used for calculating the static object matrix map.
        )doc")
        .def("rdr_get_declutter_length", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_declutter_length(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Control the time constant used by the static object removal algorithm (clutter removal algorithm), by
        changing the equivalent number of acquisitions used to compute the static object map.

        Returns:
            int: Number of frames used for calculating the static object matrix map.
        )doc")

        .def("rdr_set_elab", &RadarHandler::rdr_set_elab, "elab"_a, R"doc(
        Configure the processing level for the returned radar data.
        RAW: Raw (unfiltered) data.
        MTI: Data filtered by the moving target indicator (decluttering) algorithm.

        Args:
            elab (rdrElabTypes): Desired type of processing.
        )doc")
        .def("rdr_get_elab", [](RadarHandler &self) {
            rdrElabTypes_t y = RT_UNKNOWN;
            int ret = self.rdr_get_elab(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Configure the processing level for the returned radar data.
        RAW: Raw (unfiltered) data.
        MTI: Data filtered by the moving target indicator (decluttering) algorithm.

        Returns:
            rdrElabTypes: Desired type of elaboration.
        )doc")
        .def("rdr_set_optproc", &RadarHandler::rdr_set_optproc, "opt0"_a, "opt1"_a, R"doc(
        This command controls the streams processing steps, executed for each frame.

        Args:
            opt0, opt1 (Byte): Two bytes used to configure low-level settings.
        )doc")
        .def("rdr_get_optproc", [](RadarHandler &self) {
            uint8_t x=0;
            uint8_t y=0;
            int ret = self.rdr_get_optproc(&x, &y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(x, y);
        }, R"doc(
        This command controls the streams processing steps, executed for each frame.

        Returns:
            tuple[int, int]: A tuple containing the two bytes used to control low-level features.
        )doc")

        .def("rdr_set_sequence", &RadarHandler::rdr_set_sequence, "txSeqMask"_a, "rxSeqMask"_a, "numSeq"_a, R"doc(
        Controls the antenna channels activation for each sequence. The command has two parameters that are
        two arrays, both same dimensions, encoding the active channels for each acquisition.

        Args:
            txSeqMask (int): Transmit antennas selection
            rxSeqMask (int): Recevier antenna selection
            numSeq (int): Number of sequence
        )doc")
        .def("rdr_get_sequence", [](RadarHandler &self) {
            uint8_t x[16] = {0};
            uint8_t y[16] = {0};
            uint8_t z = 0;
            int ret = self.rdr_get_sequence(x, y, &z);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            std::vector<uint8_t> tx_valid(x, x + z);
            std::vector<uint8_t> rx_valid(y, y + z);
            return nb::make_tuple(tx_valid, rx_valid, z);
        }, R"doc(
        Controls the antenna channels activation for each sequence. The command has two parameters that are
        two arrays, both same dimensions, encoding the active channels for each acquisition.

        Returns:
            tuple[int, int, int]: A tuple containing (txSeqMask, rxSeqMask, numSeq), representing the two bytes
            for the current transmit and receive antenna selections, respectively, and the number of sequence.
        )doc")

        .def("rdr_set_carrier_frequency", &RadarHandler::rdr_set_carrier_frequency, "carrierMHz"_a, R"doc(
        Controls the center frequency of the pulses.

        Args:
            carrierMHz (int): center frequency of the pulses in MHz.
        )doc")
        .def("rdr_get_carrier_frequency", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_carrier_frequency(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Controls the center frequency of the pulses.

        Returns:
            int: center frequency of the pulses in MHz.
        )doc")

        .def("rdr_set_pulse_bandwidth", &RadarHandler::rdr_set_pulse_bandwidth, "bwMHz"_a, R"doc(
        This function controls the bandwidth of the transmitted pulse. It is recommended to set
        the bwmode parameter before changing the bandwidth.

        Args:
            bwMHz (int): Desired bandwidth in MHz.
        )doc")
        .def("rdr_get_pulse_bandwidth", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_pulse_bandwidth(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        This function controls the bandwidth of the transmitted pulse. It is recommended to set
        the bwmode parameter before changing the bandwidth.

        Returns:
            int: Current bandwidth in MHz.
        )doc")

        .def("rdr_set_bwmode", &RadarHandler::rdr_set_bwmode, "bwmode"_a, R"doc(
        Change the operating mode of the device. FW provides two different operating modes, one is a tradeoff
        between power consumption and resolution, the second designed for high resolution applications. This
        command affects configurability range and output of some parameters; this command must be the first
        command to call after the stop.

        Args:
            bwmode (int): Desired operating mode of the device.
        )doc")
        .def("rdr_get_bwmode", [](RadarHandler &self) {
            uint8_t y=0;
            int ret = self.rdr_get_bwmode(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Change the operating mode of the device. FW provides two different operating modes, one is a tradeoff
        between power consumption and resolution, the second designed for high resolution applications. This
        command affects configurability range and output of some parameters; this command must be the first
        command to call after the stop.

        Returns:
            int: Current operating mode of the device.
        )doc")

        .def("rdr_set_code", &RadarHandler::rdr_set_code, R"doc(
        Controls the codeword sent in the burst. The radar can be configured to send several pulses per burst
        from 1 up to 32. The pulses are ternary encoded, and this parameter controls the number of pulses and
        the amplitude of each pulse.

        Args:
            code (int): Desired impulse sequence.
            size (int): Length of the impulse sequence.
        )doc")
        .def("rdr_get_code", [](RadarHandler &self) {
            int8_t x[32];
            int y;
            int ret = self.rdr_get_code(x, &y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(x, y);
        }, R"doc(
        Controls the codeword sent in the burst. The radar can be configured to send several pulses per burst
        from 1 up to 32. The pulses are ternary encoded, and this parameter controls the number of pulses and
        the amplitude of each pulse.

        Return:
            tuple[int, int]: A tuple containing the code and size of the impulse sequence, respectively.
        )doc")

        .def("rdr_set_gain", &RadarHandler::rdr_set_gain, R"doc(
        Controls the variable gain amplification used by the receiver chain, this command allows the independent
        control of both the branch es (phase/quadrature) of the receive.

        Args:
            gainI, gainQ:  Desired VGA In-phase and Quadrature channel, respectively.
        )doc")
        .def("rdr_get_gain", [](RadarHandler &self) {
            uint16_t x;
            uint16_t y;
            int ret = self.rdr_get_gain(&x, &y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(x, y);
        }, R"doc(
        Controls the variable gain amplification used by the receiver chain, this command allows the independent
        control of both the branch es (phase/quadrature) of the receive.

        Returns:
            tuple[int, int]: A tuple containing VGA In-phase and Quadrature channel, respectively.
        )doc")

        .def("rdr_get_adcfreq", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_adcfreq(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        This function returns the current ADC sampling frequency, expressed in MHz . The returned value
        depends on the bwmode setting. It’s suggested to get the ADC frequency range once completed all the
        initial parametrization.

        Returns:
            fMhz: Current ADC sampling rate in MHz
        )doc")

        .def("rdr_get_antConfig",  [](RadarHandler &self) {
            txAntDesc_t tx;
            rxAntDesc_t rx;
            int ret = self.rdr_get_antConfig(&tx, &rx);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(tx, rx);
        }, R"doc(
        This command returns the module’s antenna array description. The antenna’s topology is encoded in 2
        different arrays of structures.

        Returns:
            tuple[txAntDesc, rxAntDesc]: A tuple, representing the two antennas.
        )doc")

        .def("rdr_set_canvas_2d", nb::overload_cast<canvas2d_param_t>(&RadarHandler::rdr_set_canvas), "canvas2d"_a, R"doc(
        Controls the reconstruction boundaries used by the embedded image calculator algorithm.

        Args:
            canvas2d (canvas2d_param): Desired reconstruction boundaries.
        )doc")
        .def("rdr_get_canvas_2d", [](RadarHandler &self) {
            canvas2d_param_t y;
            int ret = self.rdr_get_canvas(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Controls the reconstruction boundaries used by the embedded image calculator algorithm.

        Returns:
            canvas2d_param: Desired reconstruction boundaries.
        )doc")
        /*
        .def("rdr_set_canvas_3d", nb::overload_cast<canvas3d_param_t>(&RadarHandler::rdr_set_canvas), "canvas3d"_a, R"doc(
        Controls the reconstruction boundaries used by the embedded image calculator algorithm.

        Args:
            canvas3d (canvas3d_param_t): Desired reconstruction boundaries.
        )doc")
        .def("rdr_get_canvas_3d", [](RadarHandler &self) {
            canvas3d_param_t y;
            int ret = self.rdr_get_canvas(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Controls the reconstruction boundaries used by the embedded image calculator algorithm.

        Returns:
            canvas3d (canvas3d_param): Desired reconstruction boundaries.
        )doc")
        */

        .def("rdr_set_data_fmt", &RadarHandler::rdr_set_data_fmt, "fmt"_a, R"doc(
        Controls the data encoding used during streams data exchange. This command is mainly used for data
        compression purposes to speed-up transmission of the data over UART.

        Args:
            fmt (int): Set the format encoding used for transmitting
            streams’ data.
        )doc")
        .def("rdr_get_data_fmt", [](RadarHandler &self) {
            uint8_t y=0;
            int ret = self.rdr_get_data_fmt(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Controls the data encoding used during streams data exchange. This command is mainly used for data
        compression purposes to speed-up transmission of the data over UART.

        Returns:
            int: Set the format encoding used for transmitting streams’ data.
        )doc")

        .def("rdr_set_baudrate", &RadarHandler::rdr_set_baudrate, R"doc(
            Set the desired baudrate.

        Args:
            br (int): Desired baudrate.
        )doc")

        .def("rdr_set_multistreammode", &RadarHandler::rdr_set_multistreammode, "nstream"_a, R"doc(
        This command configures the number of acquisitions sent for every data request. By default, the
        parameter related to this command is set to 1 (1 acquisition per request). If UART channel is used,
        sending more than one acquisition per request speeds up the equivalent frame rate achievable at host
        level.

        Args:
            nstream (int): Desired number of acquisitions per request, values
            range from 1 up to 10.
        )doc")
        .def("rdr_get_multistreammode", [](RadarHandler &self) {
            uint16_t y=0;
            int ret = self.rdr_get_multistreammode(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        This command configures the number of acquisitions sent for every data request. By default, the
        parameter related to this command is set to 1 (1 acquisition per request). If UART channel is used,
        sending more than one acquisition per request speeds up the equivalent frame rate achievable at host
        level.

        Returns:
            int: Current number of acquisitions per request.
        )doc")

        .def("rdr_conv_mask2sequence", [](RadarHandler &self, std::vector<uint8_t> txMask, std::vector<uint8_t> rxMask,  int num) {
            scanSequence_t seq;
            int state = self.rdr_conv_mask2sequence(txMask.data(), rxMask.data(), &seq, num);

            if (state != 0) {
                throw std::runtime_error("Communication problem");
            }

            return seq;
        }, "txMask"_a, "rxMask"_a, "num"_a, R"doc(
        This command takes two mask vectors and combines them to create a scan sequence.

        Args:
            txMask (int): Transmit mask.
            rxMask (int): Receiver mask.
            num (int): Number of antenna combinations.

        Returns:
            scanSequence: The generated scan sequence.
        )doc")

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
        }, R"doc(
        This function requests to acquire data and create the image.

        Returns:
            image (ndarray): The created image from the acquired data.
        )doc")

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
        }, R"doc(
        This function requests to acquire data.

        Returns:
            ndarray: Acquired data.
        )doc")

        // new
        .def("rdr_set_profile", &RadarHandler::rdr_set_profile, "profile"_a R"doc(
        This command sets the profile to be configured; this command changes multiple parameters to set the
        device according to the selected profile.

        Returns:
            profile (profile): Information about distance and resolution.
        )doc")

        .def("rdr_set_staticObjRemoval", &RadarHandler::rdr_set_staticObjRemoval, "enable"_a, R"doc(
        Controls the execution of the static object removal algorithm

        Args:
            enable (int): Enable/disable the execution of the algorithm.
        )doc")
        .def("rdr_get_staticObjRemoval", [](RadarHandler &self){
            uint8_t y=0;
            int ret = self.rdr_get_staticObjRemoval(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
                }
            return y;
        }, R"doc(
        Controls the execution of the static object removal algorithm

        Returns:
            int : Current Enable/disable the execution of the algorithm.
        )doc")

        .def("rdr_set_staticObjMapUpdateTime", &RadarHandler::rdr_set_staticObjMapUpdateTime, "timeConstant"_a, R"doc(
        Controls the time- constant used by the static object removal algorithm to update the static object map
        matrix.

        Args:
            timeConstant (float): Desired time constant in seconds.
        )doc")
        .def("rdr_get_staticObjMapUpdateTime", [](RadarHandler &self){
            float y=0;
            int ret = self.rdr_get_staticObjMapUpdateTime(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Controls the time- constant used by the static object removal algorithm to update the static object map
        matrix.

        Returns:
            float: Current time constant in seconds.
        )doc")

        .def("rdr_set_rxGain", &RadarHandler::rdr_set_rxGain, "val"_a, R"doc(
        Controls the gain used by the receiver chain.

        Args:
            val (int): Desired gain level.
        )doc")
        .def("rdr_get_rxGain", [](RadarHandler &self){
            uint8_t y=0;
            int ret = self.rdr_get_rxGain(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Controls the gain used by the receiver chain.

        Returns:
            int: Current gain level.
        )doc")

        .def("rdr_set_embeddedImgAlgo", &RadarHandler::rdr_set_embeddedImgAlgo, "enable"_a, R"doc(
        Controls the execution of the algorithm responsible for computing the output image (2D devices only).
        When this algorithm is activated, the embedded processor combines the streams and calculates the
        output image.

        Args:
            enable (int): Enabling state of the device.
        )doc")
        .def("rdr_get_embeddedImgAlgo", [](RadarHandler &self){
            uint8_t y=0;
            int ret = self.rdr_get_embeddedImgAlgo(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Controls the execution of the algorithm responsible for computing the output image (2D devices only).
        When this algorithm is activated, the embedded processor combines the streams and calculates the
        output image.

        Returns:
            int: Enabling state of the device.
        )doc")

        .def("rdr_get_version_fw", [](RadarHandler &self){
            RdrFWVer_t y;
            int ret = self.rdr_get_version_fw(&y);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return y;
        }, R"doc(
        Information about the implemented firmware. Short version.

        Returns:
            RdrFWVer: Firmware information.
        )doc")

        .def("rdr_set_range", &RadarHandler::rdr_set_range, "min"_a, "max"_a, R"doc(
        Controls the boundaries of the region of interest (ROI) expressed in meters.

        Args:
            min (float): Set the minimum distance of the ROI.
            max (float): Set the maximum distance of the ROI.
        )doc")
        .def("rdr_get_range", [](RadarHandler &self){
            float pmin;
            float pmax;
            int ret = self.rdr_get_range(&pmin, &pmax);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return nb::make_tuple(pmin, pmax);
        }, R"doc(
        Controls the boundaries of the region of interest (ROI) expressed in meters.

        Return:
            float: Get the current minimum distance of the ROI.
            max (float): Get the current maximum distance of the ROI.
        )doc")
        .def("rdr_get_versExtended", [](RadarHandler &self){
            string pstr;
            uint8_t type;
            int ret = self.rdr_get_versExtended(&pstr, type);
            if(ret){
                throw std::runtime_error("Communication problem");
            }
            return pstr;
        }, R"doc(
        Information about the implemented firmware. Extended version.

        Return:
            RdrFW_ex (string): Firmware information.
        )doc");
}

