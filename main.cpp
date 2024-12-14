#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>
#include <SFML/Audio.hpp>
#include <fftw3.h>
#include <cmath> // sqrt

int main(int argc, char** argv){
  auto window = std::make_unique<sf::RenderWindow>(
    sf::VideoMode(1280,720), 
    "Audio Spectrum",
    sf::Style::Titlebar | sf::Style::Close
  ); 

  if(argc < 2){
    std::cerr << "Usage: " << argv[0] << " file.[mp3|wav]\n";
    return EXIT_FAILURE;
  }

  sf::SoundBuffer buffer;
  if(!buffer.loadFromFile(argv[1])){
    std::cerr << "Failed to load audio file.\n";
    return EXIT_FAILURE;
  }

  sf::Sound sound(buffer);
  sound.play();

  const int sample_size = 1024;

  fftw_complex * in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * sample_size);
  fftw_complex * out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * sample_size);

  fftw_plan plan = fftw_plan_dft_1d(sample_size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

  std::vector<float> spectrum(sample_size / 2);

  const sf::Int16 * samples = buffer.getSamples();

  std::size_t sample_count = buffer.getSampleCount();

  std::size_t channels = buffer.getChannelCount();

  std::size_t current_sample = {};

  sf::Texture texture;
  texture.loadFromFile("./bg.png");
  sf::Sprite sprite(texture);

  sf::Font font;
  font.loadFromFile("./andarilho-font.ttf");
  std::string music = argv[1];
  music = music.substr(0, music.length() - 4);
  sf::Text text(music, font, 18);
  text.setPosition(20.f, 10.f);

  while(window->isOpen()){
    auto event = std::make_unique<sf::Event>();
    while(window->pollEvent(*event)){
      if(event->type == sf::Event::Closed){
        window->close();
      }
    }

    if(sound.getStatus() == sf::Sound::Stopped){
      break;
    }

    for(int i = 0; i < sample_size; ++i){
      std::size_t index = (current_sample + i) % (sample_count / channels);
      in[i][0] = samples[index * channels] / 32768.0;
      in[i][1] = 0.0;
    }

    fftw_execute(plan);

    for(int i = {}; i < sample_size / 2; ++i){
      spectrum[i] = std::sqrt(out[i][0] * out[i][0] + out[i][1] + out[i][1]);
    }

    current_sample += sample_size;

    window->clear();
    window->draw(sprite);
    window->draw(text);

    for(int i = {}; i < 50; i++){
      sf::RectangleShape bar;
      bar.setSize(sf::Vector2f(2, spectrum[i] * 1.f));
      bar.setPosition(
          i * 6 + window->getPosition().x / 2.f + 480.f, 
          window->getPosition().y / 2.f + 300.f);
      bar.setRotation(180);
      window->draw(bar);


      bar.setSize(sf::Vector2f(2, -(spectrum[i] * 1.f)));
      window->draw(bar);
    }

    for(int i = 49; i >= 0; --i){
      sf::RectangleShape bar;
      bar.setSize(sf::Vector2f(2, spectrum[i] * 1.f));
      bar.setPosition(
          (49 - i) * 6 + 340.f, 
          window->getPosition().y / 2.f + 300.f);
      bar.setRotation(180);
      window->draw(bar);

      bar.setSize(sf::Vector2f(2, -(spectrum[i] * 1.f)));
      window->draw(bar);
    }

    window->display();
  }

  fftw_destroy_plan(plan);
  fftw_free(in);
  fftw_free(out);

  return EXIT_SUCCESS;
}
// g++ main.cpp -lsfml-graphics -lsfml-window -lsfml-system
