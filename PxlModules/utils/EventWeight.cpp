#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

#include <string>
#include <unordered_map>

static pxl::Logger logger("EventWeight");

struct FileInfo
{
    double nEvents;
    double crossSection;
};

const std::unordered_map<std::string,FileInfo> eventWeights = {
    {"TToLeptons_t-channel-CSA14_Tune4C_13TeV-aMCatNLO-tauola",
        {
            1037660,
            136.02*0.324
        }
    },
    {"TBarToLeptons_t-channel_Tune4C_CSA14_13TeV-aMCatNLO-tauola",
        {
            519948,
            80.95*0.324
        }
    },

    {"T_tW-channel-DR_Tune4C_13TeV-CSA14-powheg-tauola",
        {
            986100,
            35.6
        }
    },
    {"Tbar_tW-channel-DR_Tune4C_13TeV-CSA14-powheg-tauola",
        {
            971800,
            35.6
        }
    },

    {"TToLeptons_s-channel-CSA14_Tune4C_13TeV-aMCatNLO-tauola",
        {
            500000,
            7.16 * 0.324
        }
    },
    {"TBarToLeptons_s-channel-CSA14_Tune4C_13TeV-aMCatNLO-tauola",
        {
            250000,
            4.20 * 0.324
        }
    },

    {"TTJets_MSDecaysCKM_central_Tune4C_13TeV-madgraph-tauola",
        {
            25446993,
            831.76
        }
    },
    {"WJetsToLNu_13TeV-madgraph-pythia8-tauola",
        {
            10017462,
            20508.9*3 //(= n leptons)
        }
    },
    {"DYJetsToLL_M-50_13TeV-madgraph-pythia8",
        {
            2829164,
            6025.2
        }
    },

    {"QCD_Pt-20toInf_MuEnrichedPt15_PionKaonDecay_Tune4C_13TeV_pythia8",
        {
            4777926,
            866600000 * 0.00044
        }
    },

    {"QCD_Pt_20to30_bcToE_Tune4C_13TeV_pythia8",
        {
            999926,
            675900000 * 0.00075
        }
    },
    {"QCD_Pt_30to80_bcToE_Tune4C_13TeV_pythia8",
        {
            1852166,
            185900000 * 0.00272
        }
    },
    {"QCD_Pt_80to170_bcToE_Tune4C_13TeV_pythia8",
        {
            1000671,
            3495000 * 0.01225
        }
    },
    {"QCD_Pt_170toInf_bcToE_Tune4C_13TeV_pythia8",
        {
            1000221,
            128500 * 0.0406
        }
    },

    {"QCD_Pt-20to30_EMEnriched_Tune4C_13TeV_pythia8",
        {
            1987127,
            677300000 * 0.007
        }
    },
    {"QCD_Pt-30to80_EMEnriched_Tune4C_13TeV_pythia8",
        {
            2000838,
            185900000 * 0.056
        }
    },
    {"QCD_Pt-80to170_EMEnriched_Tune4C_13TeV_pythia8",
        {
            1959507,
            3529000 * 0.158
        }
    },

    {"WJetsToLNu_HT-100to200_Tune4C_13TeV-madgraph-tauola",
        {
            5262265,
            1817
        }
    },
    {"WJetsToLNu_HT-200to400_Tune4C_13TeV-madgraph-tauola",
        {
            4936077,
            471.6
        }
    },
    {"WJetsToLNu_HT-400to600_Tune4C_13TeV-madgraph-tauola",
        {
            4640594,
            55.61
        }
    },
    {"WJetsToLNu_HT-600toInf_Tune4C_13TeV-madgraph-tauola",
        {
            4581841,
            18.81
        }
    }

};

class EventWeight:
    public pxl::Module
{
    private:
        pxl::Source* _outputSource;

        std::string _processNameField;

    public:
        EventWeight():
            Module(),
            _processNameField("ProcessName")
        {
            addSink("input", "input");
            _outputSource = addSource("output","output");
            
            addOption("name of process field","",_processNameField);
        }

        ~EventWeight()
        {
        }

        // every Module needs a unique type
        static const std::string &getStaticType()
        {
            static std::string type ("EventWeight");
            return type;
        }

        // static and dynamic methods are needed
        const std::string &getType() const
        {
            return getStaticType();
        }

        bool isRunnable() const
        {
            // this module does not provide events, so return false
            return false;
        }

        void initialize() throw (std::runtime_error)
        {
        }

        void beginJob() throw (std::runtime_error)
        {
            getOption("name of process field",_processNameField);
        }

        bool analyse(pxl::Sink *sink) throw (std::runtime_error)
        {
            try
            {
                pxl::Event *event  = dynamic_cast<pxl::Event*>(sink->get());
                if (event)
                {
                    std::string processName = event->getUserRecord(_processNameField).toString();
                    auto it = eventWeights.find(processName);
                    if (it==eventWeights.end())
                    {
                        logger(pxl::LOG_LEVEL_ERROR,"no event weight information available for process name '"+processName+"'");
                        //throw std::runtime_error(getName()+": no event weight information available for process name '"+processName+"'");
                    }
                    else
                    {
                        event->setUserRecord("mc_weight",1.0*it->second.crossSection/it->second.nEvents);
                    }

                    _outputSource->setTargets(event);
                    return _outputSource->processTargets();
                }
            }
            catch(std::exception &e)
            {
                throw std::runtime_error(getName()+": "+e.what());
            }
            catch(...)
            {
                throw std::runtime_error(getName()+": unknown exception");
            }

            logger(pxl::LOG_LEVEL_ERROR , "Analysed event is not an pxl::Event !");
            return false;
        }

        void shutdown() throw(std::runtime_error)
        {
        }

        void destroy() throw (std::runtime_error)
        {
            delete this;
        }
};

PXL_MODULE_INIT(EventWeight)
PXL_PLUGIN_INIT
