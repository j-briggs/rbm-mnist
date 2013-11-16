#include "mnist.h"
#include "rbm.h"

/** Algorithm from Earl F. Glynn's web page:
* <a href="http://www.efg2.com/Lab/ScienceAndEngineering/Spectra.htm">Spectra Lab Report</a>
* */
//#include "spectrum.inl"

//#include <Magick++.h>

// MacOS X
// clang++ -g -O2 -std=c++11 -stdlib=libc++ -I/usr/local/include/GraphicsMagick -o dbn demo.cc /Users/jack/Downloads/GraphicsMagick-1.3.18/Magick++/lib/.libs/libGraphicsMagick++.a  /Users/jack/Downloads/GraphicsMagick-1.3.18/magick/.libs/libGraphicsMagick.a -lz -lxml2 -lpng -ljpeg -lbz2
// Ubuntu
// clang++ --std=c++0x -o dbn -g -O2 demo.cc -lz -I /usr/include/GraphicsMagick -lGraphicsMagick++

// training and testing functions
void train_dbn_simple(std::vector<Vector>& inputs, std::vector<Vector>& targets, int data_size) {
  DeepBeliefNet dbn;
  std::vector<int> layers;
  layers.push_back(data_size);
  layers.push_back(300);
  layers.push_back(300);
  layers.push_back(500);
  std::vector<int> adjust;
  adjust.push_back(0);
  adjust.push_back(0);
  adjust.push_back(10);
  dbn.build(layers, adjust);

  LRBM::Conf conf;
  conf.max_epoch_ = 6; conf.max_batches_ = 100; conf.batch_size_ = 100;

  dbn.train(inputs, targets, dbn.max_layer(), conf);

  std::ofstream f("dbn-s.dat", std::ofstream::binary);
  dbn.store(f);
}

void train_dbn(std::vector<Vector>& inputs, std::vector<Vector>& targets, int data_size) {
  DeepBeliefNet dbn;
  std::vector<int> layers;
  layers.push_back(data_size);
  layers.push_back(300);
  layers.push_back(300);
  layers.push_back(500);
  layers.push_back(10);
  dbn.build(layers);
  auto& rbm = dbn.output_layer();
  rbm->type_ = RBM::EXP;

  std::default_random_engine eng(::time(NULL));
  std::normal_distribution<double> rng(0.0, 1.0);

  LRBM::Conf conf;

  bool resume = false;
  if (resume) {
    std::ifstream f("dbn.dat", std::ifstream::binary);
    dbn.load(f);
    conf.max_epoch_ = 2; conf.max_batches_ = 300; conf.batch_size_ = 200;
  }
  else {
    conf.max_epoch_ = 10; conf.max_batches_ = 300; conf.batch_size_ = 200;
    dbn.pretrain(inputs, conf);
  }

  conf.max_epoch_ = 10; conf.max_batches_ /= 5; conf.batch_size_ *= 5;
  dbn.backprop(inputs, targets, conf);

  std::ofstream f("dbn.dat", std::ofstream::binary);
  dbn.store(f);
}

void train_autoencoder(std::vector<Vector>& inputs, int data_size) {
  AutoEncoder enc;
  std::vector<int> layers;
  layers.push_back(data_size);
  layers.push_back(500);
  layers.push_back(30);
  layers.push_back(500);
  layers.push_back(data_size);
  enc.build(layers);

  auto& rbm = enc.rbms_[enc.max_layer() / 2 - 1];
  rbm->type_ = RBM::LINEAR;

  LRBM::Conf conf;
  conf.max_epoch_ = 10; conf.max_batches_ = 50; conf.batch_size_ = 100;
  enc.pretrain(inputs, conf);

  conf.max_epoch_ = 10; conf.max_batches_ /= 5; conf.batch_size_ *= 5;
  enc.backprop(inputs, conf);

  std::ofstream f("enc.dat", std::ofstream::binary);
  enc.store(f);
}

void test_dbn(std::vector<Sample>& samples, std::vector<Vector>& inputs, bool is_simple) {
  DeepBeliefNet rbm;
  std::string file = is_simple? "dbn-s.dat" : "dbn.dat";
  std::ifstream f(file, std::ifstream::binary);
  rbm.load(f);

  size_t correct = 0, second = 0;
  for (size_t i = 0; i < samples.size(); ++i) {
    const Sample& sample = samples[i];

    std::vector<int> idx(10);
    for(int t=0; t<10; ++t) idx[t] = t;

    static Vector nil;
    Vector output(10);
    if (is_simple)
      rbm.predict(inputs[i], output, nil);
    else
      rbm.predict(inputs[i], nil, output);

    std::sort(idx.begin(), idx.end(), [&output](int x, int y) { return output[x] > output[y]; });

    if (idx[0] == (int)sample.label_) ++ correct;
    else if (idx[1] == (int)sample.label_) ++ second;


    if ((i + 1) % 100 == 0)	std::cout << "# " << correct << "/" << i + 1 << " recognized. 1st: " << (correct * 100.0/ (i+1)) << "%, 1st+2nd: " << (correct + second) * 100.0/(i+1) << "%" << std::endl;
  }

  std::cout << "# " << correct << " recognized." << std::endl;
}

int main(int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << "<train-simple|train|test> <image-file> <label-file>" << std::endl;
    return -1;
  }

  std::vector<Sample> samples;
  int n = mnist::read(argv[2], argv[3], samples);
  if (n <= 0) {
    std::cerr << "failed to read mnist data files: " << argv[2] << " ," << argv[3] << std::endl;
    return -1;
  }

  std::string command = argv[1];

  // initialize data
  int data_size = samples[0].data_.size();
  std::vector<Vector> inputs(n);
  std::vector<Vector> targets(n);
  for (size_t i=0; i< n; ++i) {
    const Sample& sample = samples[i];
    Vector& input = inputs[i];
    Vector& target = targets[i];

    input.resize(data_size); target.resize(10);
    for (size_t j=0; j<data_size; ++j) input[j] = sample.data_[j] / 255.0f; // > 30 ? 1.0: 0.0; // binary 
    target[sample.label_] = 1.0;
  }

  // execute commands
  if (command == "train") train_dbn(inputs, targets, data_size);
  else if (command == "train-simple") train_dbn_simple(inputs, targets, data_size);
  else if (command == "test") test_dbn(samples, inputs, false);
  else if (command == "test-simple") test_dbn(samples, inputs, true);
  else if (command == "train-encoder") train_autoencoder(inputs, data_size);
  else {
    std::cerr << "unrecognized command: " << command << std::endl;	
  }

  return 0;
}


