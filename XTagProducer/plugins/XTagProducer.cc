// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "LLPReco/DataFormats/interface/XTagInfo.h"

#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"
#include "FWCore/Framework/interface/makeRefToBaseProdFrom.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "LLPReco/XTagProducer/interface/tensor_fillers.h"



class XTagProducer : public edm::stream::EDProducer<> {
   public:
      explicit XTagProducer(const edm::ParameterSet&);
      ~XTagProducer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      const edm::EDGetTokenT<std::vector<reco::XTagInfo>> _src;
      tensorflow::GraphDef* _graphDef;
      std::vector<std::pair<std::string, double>> _ctau_pairs;
      tensorflow::Session* _session;

      virtual void beginStream(edm::StreamID) override;
      virtual void produce(edm::Event&, const edm::EventSetup&) override;
      virtual void endStream() override;
};

XTagProducer::XTagProducer(const edm::ParameterSet& iConfig)
    : _src(consumes<std::vector<reco::XTagInfo>>(iConfig.getParameter<edm::InputTag>("src"))),
     _graphDef(tensorflow::loadGraphDef(iConfig.getParameter<edm::FileInPath>("graph_path").fullPath())),
    _session(nullptr)
{

    std::vector<double> _ctau_values = iConfig.getParameter<std::vector<double>>("ctau_values");
    std::vector<std::string> _ctau_descriptors = iConfig.getParameter<std::vector<std::string>>("ctau_descriptors");
     for (unsigned int i = 0; i < _ctau_values.size(); i++){
         _ctau_pairs.emplace_back(std::pair(_ctau_descriptors.at(i), _ctau_values.at(i) ));
     }
    tensorflow::setLogging("3");
     // get threading config and build session options
    tensorflow::SessionOptions sessionOptions;
    //tensorflow::setThreading(sessionOptions, 1, "no_threads");


    // create the session using the meta graph from the cache
    _session = tensorflow::createSession(_graphDef, sessionOptions); 
    for (const auto& ctau_pair : _ctau_pairs) {
        produces<reco::JetTagCollection>(ctau_pair.first);
    }

}


XTagProducer::~XTagProducer()
{
      if (_session != nullptr) {
          tensorflow::closeSession(_session);
          _session = nullptr;
      }
      delete _graphDef;
      _graphDef = nullptr;
}


void
XTagProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    edm::Handle<std::vector<reco::XTagInfo>> tag_infos;
    iEvent.getByToken(_src, tag_infos);

    // initialize output collection
    std::vector<std::unique_ptr<reco::JetTagCollection>> output_tags;
    for (unsigned int ictau = 0; ictau < _ctau_pairs.size(); ictau++){
        if (!tag_infos->empty()) {
          auto jet_ref = tag_infos->begin()->jet();
          output_tags.emplace_back(std::make_unique<reco::JetTagCollection>(edm::makeRefToBaseProdFrom(jet_ref, iEvent)));
        } 
        else {
          output_tags.emplace_back(std::make_unique<reco::JetTagCollection>());
        }
    }


    std::vector<std::string> input_names_{"globalvars", "cpf", "npf", "sv", "gen"};
    unsigned int ntags = tag_infos->size();

    std::vector<tensorflow::TensorShape> input_sizes{
        {ntags, 14},
        {ntags, 25, 18},
        {ntags, 25, 6},
        {ntags, 4, 12},
        {ntags, 1},  
      };

    tensorflow::NamedTensorList _input_tensors;
    _input_tensors.resize(input_sizes.size());

    for (unsigned int i = 0; i < input_names_.size(); i++) {
        std::string group_name = input_names_[i];
        tensorflow::Tensor group_tensor(tensorflow::DT_FLOAT, input_sizes[i]);
        _input_tensors[i] = tensorflow::NamedTensor(group_name, group_tensor);
    }

    // Fill with zeros
    for (unsigned int i = 0; i < input_sizes.size(); i++) {
        _input_tensors[i].second.flat<float>().setZero();
    }

    // fill values of the input tensors
    for (unsigned int itag= 0; itag < ntags; itag++) {
        const auto& features = tag_infos->at(itag).features();
        //{"gen", "globalvars", "cpf", "npf", "sv"};
        // keep ctau at 1 mm for now
        auto cpf = features.cpf_features;
        unsigned int ncpf = std::min((unsigned int)cpf.size(), (unsigned int)25);
        auto npf = features.npf_features;
        unsigned int nnpf = std::min((unsigned int)npf.size(), (unsigned int)25);
        auto sv = features.sv_features;
        unsigned int nsv = std::min((unsigned int)sv.size(), (unsigned int)4);

        jet_tensor_filler(_input_tensors.at(0).second, itag, features);


        for (unsigned int i = 0; i < ncpf; i++){
            cpf_tensor_filler(_input_tensors.at(1).second, itag, i, cpf.at(i));
        }


        for (unsigned int i = 0; i < nnpf; i++){
            npf_tensor_filler(_input_tensors.at(2).second, itag, i, npf.at(i));
        }

        for (unsigned int i = 0; i < nsv; i++){
            sv_tensor_filler(_input_tensors.at(3).second, itag, i, sv.at(i));
        }
    }

    /*
    auto cpfs = _input_tensors.at(1).second.shaped<float, 3>({ntags, 25, 18});
    auto npfs = _input_tensors.at(2).second.shaped<float, 3>({ntags, 25, 6});
    auto svs = _input_tensors.at(3).second.shaped<float, 3>({ntags, 4, 12});
    for (unsigned int itag= 0; itag < ntags; itag++) {
        std::cout << "tag number: " << itag << std::endl;

        for (unsigned int i = 0; i < 25; i++){
            std::cout << "cand number: " << i;
            for (unsigned int j = 0; j < 18; j++){
                std::cout << cpfs(itag, i, j) << " ";
            }
            std::cout << std::endl;
        }

        for (unsigned int i = 0; i < 25; i++){
            std::cout << "cand number: " << i;
            for (unsigned int j = 0; j < 6; j++){
                std::cout << npfs(itag, i, j) << " ";
            }
            std::cout << std::endl;
        }
        for (unsigned int i = 0; i < 4; i++){
            std::cout << "cand number: " << i;
            for (unsigned int j = 0; j < 12; j++){
                std::cout << svs(itag, i, j) << " ";
            }
            std::cout << std::endl;
        }
    }
    */

    for (unsigned int ictau = 0; ictau < _ctau_pairs.size(); ictau++){
        double ctau = _ctau_pairs.at(ictau).second;
        for (unsigned int itag = 0; itag < ntags; itag++) {
             float *ptr = &(_input_tensors.at(4).second).matrix<float>()(itag, 0);
             *ptr = ctau;
        }

        std::vector<tensorflow::Tensor> outputs;
        tensorflow::run(_session, _input_tensors, {"prediction"}, &outputs);
        auto scores = outputs[0].matrix<float>();

        for (unsigned int itag = 0; itag < ntags; itag++) {
            const auto& jet_ref = tag_infos->at(itag).jet();
            //std::cout << "tag number: " << scores(itag, 0) << ", " << scores(itag, 1) << ", " << scores(itag, 2) << ", " << scores(itag, 3) << ", " << scores(itag, 4) << ", " << std::endl;
            (*(output_tags.at(ictau)))[jet_ref] = scores(itag, 4); // LLP probability
        }

        iEvent.put(std::move(output_tags[ictau]), _ctau_pairs.at(ictau).first);
    }
    //_session->reset();
}

void
XTagProducer::beginStream(edm::StreamID)
{
}

void
XTagProducer::endStream() {
}
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
XTagProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("src", edm::InputTag("pfXTagInfos"));
  desc.add<edm::FileInPath>("graph_path", edm::FileInPath("LLPReco/XTagProducer/data/da.pb"));
  desc.add<std::vector<double>>("ctau_values", std::vector<double>({-2., 0., 3.}));
  desc.add<std::vector<std::string>>("ctau_descriptors", std::vector<std::string>({"0p01", "1", "1000"}));
  descriptions.add("pfXTags", desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(XTagProducer);
