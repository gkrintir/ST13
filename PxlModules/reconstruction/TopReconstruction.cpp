#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

static pxl::Logger logger("TopReconstruction");

class TopReconstruction:
    public pxl::Module
{
    private:
        pxl::Source* _outputSource;
        
        std::string _inputEventViewName;
        std::string _leptonName;
        std::string _neutrinoName;
        std::string _bJetName;
        std::string _lightJetName;
        
        std::string _wbosonName;
        std::string _topName;

        bool _addBestTopHypothesis;
        bool _addAngles;
        
    public:
        TopReconstruction():
            Module(),
            _inputEventViewName("Reconstructed"),
            _leptonName("TightMuon"),
            _neutrinoName("Neutrino"),
            _bJetName("SelectedBJet"),
            _lightJetName("SelectedJet"),
            _wbosonName("W"),
            _topName("Top"),
            _addBestTopHypothesis(true),
            _addAngles(true)
        {
            addSink("input", "input");
            _outputSource = addSource("selected","selected");

            addOption("event view","name of the event view",_inputEventViewName);
            addOption("lepton","name of the lepton from top decay",_leptonName);
            addOption("neutrino","name of the neutrino from top decay",_neutrinoName);
            addOption("b-jet","name of the b-jet from top decay",_bJetName);
            addOption("light jet","name of the light jet from top decay (optional)",_lightJetName);
            
            addOption("W boson","name of the reconstructed W boson",_wbosonName);
            addOption("top","name of the reconstructed top",_topName);

            addOption("add bestTop","includes a best top candidate",_addBestTopHypothesis);
            addOption("add angles","calculates polarization angles",_addAngles);
        }

        ~TopReconstruction()
        {
        }

        // every Module needs a unique type
        static const std::string &getStaticType()
        {
            static std::string type ("TopReconstruction");
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
            getOption("event view",_inputEventViewName);
            getOption("lepton",_leptonName);
            getOption("neutrino",_neutrinoName);
            getOption("b-jet",_bJetName);
            getOption("light jet",_lightJetName);
            getOption("W boson",_wbosonName);
            getOption("top",_topName);

            getOption("add bestTop",_addBestTopHypothesis);
            getOption("add angles",_addAngles);
        }

        float angle(const pxl::Particle* p1, const pxl::Basic3Vector& boost1, const pxl::Particle* p2, const pxl::Basic3Vector& boost2)
        {
            pxl::LorentzVector boostP1 = p1->getVector();
            boostP1.boost(-boost1);
            pxl::LorentzVector boostP2 = p2->getVector();
            boostP2.boost(-boost2);

            return (boostP1.getPx()*boostP2.getPx()+boostP1.getPy()*boostP2.getPy()+boostP1.getPz()*boostP2.getPz())/(boostP1.getMag()*boostP2.getMag());
        }

        bool analyse(pxl::Sink *sink) throw (std::runtime_error)
        {
            try
            {
                pxl::Event *event  = dynamic_cast<pxl::Event*>(sink->get());
                if (event)
                {
                    std::vector<pxl::EventView*> eventViews;
                    event->getObjectsOfType(eventViews);
                    
                    for (unsigned ieventView=0; ieventView<eventViews.size();++ieventView)
                    {
                        pxl::EventView* eventView = eventViews[ieventView];
                        if (eventView->getName()==_inputEventViewName)
                        {
                            std::vector<pxl::Particle*> particles;
                            eventView->getObjectsOfType(particles);

                            pxl::Particle* lepton = nullptr;
                            pxl::Particle* neutrino = nullptr;
                            pxl::Particle* bjet = nullptr;
                            pxl::Particle* lightjet = nullptr;

                            for (unsigned int iparticle = 0; iparticle<particles.size(); ++iparticle)
                            {
                                pxl::Particle* particle = particles[iparticle];
                                if (!lepton and particle->getName()==_leptonName)
                                {
                                    lepton=particle;
                                }
                                if (!neutrino and particle->getName()==_neutrinoName)
                                {
                                    neutrino=particle;
                                }
                                if (!bjet and particle->getName()==_bJetName)
                                {
                                    bjet=particle;
                                }
                                if (!lightjet and particle->getName()==_lightJetName)
                                {
                                    lightjet=particle;
                                }
                            }
                            if (lepton && neutrino)
                            {
                                pxl::Particle* wboson = eventView->create<pxl::Particle>();
                                wboson->setName(_wbosonName);
                                wboson->addP4(lepton);
                                wboson->addP4(neutrino);

                                if (bjet)
                                {
                                    pxl::Particle* top = eventView->create<pxl::Particle>();
                                    top->setName(_topName);
                                    top->addP4(lepton);
                                    top->addP4(bjet);
                                    top->addP4(neutrino);

                                    if (lightjet && _addBestTopHypothesis)
                                    {
                                        pxl::Particle* bestTop = eventView->create<pxl::Particle>();
                                        bestTop->setName(_topName+"_best");
                                        bestTop->addP4(lepton);
                                        bestTop->addP4(lightjet);
                                        bestTop->addP4(neutrino);
                                        if (fabs(top->getMass()-173.0)<fabs(bestTop->getMass()-173.0))
                                        {
                                            bestTop->setP4(top->getVector());
                                        }
                                    }

                                    if (lightjet && _addAngles)
                                    {
                                        top->setUserRecord("cosTheta_lq",angle(lepton,top->getBoostVector(),lightjet,top->getBoostVector()));
                                        top->setUserRecord("cosTheta_whel",angle(lepton,wboson->getBoostVector(),wboson,top->getBoostVector()));
                                    }
                                }
                            }
                        }
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

PXL_MODULE_INIT(TopReconstruction)
PXL_PLUGIN_INIT
