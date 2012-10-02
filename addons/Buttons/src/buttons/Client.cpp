#include <buttons/Buttons.h>
#include <buttons/Client.h>
#include <buttons/Server.h>

namespace buttons {
	Client::Client(const std::string ip, int port)
		:ip(ip)
		,port(port)
	{
	}

	Client::~Client() {
	}

	void Client::start() {
		thread.create(*this);
	}

	// RUN IN SEPARATE THREAD
	void Client::run() {
		if(!sock.connect(ip.c_str(), port)) {
			printf("Error: cannot sonnect to %s:%d\n", ip.c_str(), port);
			return;
		}
		
		std::vector<char> tmp(2048);
		sock.setBlocking(false);
		while(true) {
			int bytes_read = sock.read(&tmp[0], tmp.size());
			if(bytes_read) {
				buffer.addBytes((char*)&tmp[0], bytes_read);
				parseBuffer();
			}
			// Check if there are tasks that we need to handle
			mutex.lock();
			{
				for(std::vector<CommandData>::iterator it = out_commands.begin(); it != out_commands.end(); ++it) {
					CommandData& cmd = *it;
					send(cmd.buffer.getPtr(), cmd.buffer.getNumBytes());
				}
				out_commands.clear();
			}
			mutex.unlock();
		}
	}
	void Client::send(const char* buffer, size_t len) {
		size_t bytes_left = len;
		sock.setBlocking(true);
		while(bytes_left) {
			int done = sock.send(buffer, len);
			if(done < 0) {
				break;
			}
			else {
				bytes_left -= done;
			}
		}
		sock.setBlocking(false);
	}

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
	void Client::parseBuffer() {
		do {
			if(buffer.size() < 5) { // for now each command must contain an ID (1 byte) + size (4 bytes)
				return;
			}
			unsigned int command_size = buffer.getUI32(1); // peek for the command size
			if(buffer.size() < command_size) {
				printf("Buffer is not big enough yet.., should be %u and it is %zu\n", command_size, buffer.size());
				return;
			}
			CommandData deserialized;
			if(util.deserialize(buffer, deserialized, elements)) {
				addInCommand(deserialized);
			}
		} while(buffer.size() > 0);
	}
	
	void Client::addInCommand(CommandData cmd) {
		mutex.lock();
		in_commands.push_back(cmd);
		mutex.unlock();
	}
	void Client::addOutCommand(CommandData cmd) {
		mutex.lock();
		out_commands.push_back(cmd);
		mutex.unlock();
	}

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	void Client::update() {
		mutex.lock();
		{
			std::vector<CommandData>::iterator it = in_commands.begin();
			while(it != in_commands.end()) {
				CommandData& cmd = *it;
				switch(cmd.name) {
				case BDATA_SCHEME: {
					parseScheme(cmd); 
					break;
				}
				case BDATA_SLIDERI: {
					cmd.slideri->setValue(cmd.slideri_value);
					cmd.slideri->needsRedraw();
					break;
				}
				case BDATA_SLIDERF: {
					cmd.sliderf->setValue(cmd.sliderf_value);
					cmd.sliderf->needsRedraw();			  
					break;
				}
				default: printf("Error: Unhandled in command.\n"); break;
				};
				it = in_commands.erase(it);
			}
		}
		mutex.unlock();
		for(std::map<unsigned int, buttons::Buttons*>::iterator it = buttons.begin(); it != buttons.end(); ++it) {
			it->second->update();
		}
	}

	void Client::draw() {
		for(std::map<unsigned int, buttons::Buttons*>::iterator it = buttons.begin(); it != buttons.end(); ++it) {
			it->second->draw();
		}
	}

	void Client::onMouseMoved(int x, int y) {
		for(std::map<unsigned int, buttons::Buttons*>::iterator it = buttons.begin(); it != buttons.end(); ++it) {
			it->second->onMouseMoved(x,y);
		}
	}

	void Client::onMouseUp(int x, int y) {
		for(std::map<unsigned int, buttons::Buttons*>::iterator it = buttons.begin(); it != buttons.end(); ++it) {
			it->second->onMouseUp(x,y);
		}
	}

	void Client::onMouseDown(int x, int y) {
		for(std::map<unsigned int, buttons::Buttons*>::iterator it = buttons.begin(); it != buttons.end(); ++it) {
			it->second->onMouseDown(x,y);
		}
	}
	
	
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// CREATES THE BUTTONS/PANELS FROM THE REMOTE SCHEME
	void Client::parseScheme(CommandData& task) {
		while(true) {
			char scheme_cmd = task.buffer.consumeByte();
			switch(scheme_cmd) {
			case BDATA_GUI: {
				unsigned int x = task.buffer.consumeUI32();
				unsigned int y = task.buffer.consumeUI32();
				unsigned int w = task.buffer.consumeUI32();
				unsigned int h = task.buffer.consumeUI32();
				std::string title = task.buffer.consumeString();
				unsigned short num_els = task.buffer.consumeUI16();
				
				// STORE AT ID
				buttons::Buttons* gui = new Buttons(title, w);
				gui->addListener(this);
				unsigned int buttons_id = buttons_hash(title.c_str(), title.size());
				buttons[buttons_id] = gui;

				// PARSE ALL ELEMENTS
				for(int i = 0; i < num_els; ++i) {

					// COMMON/SHARED VALUES
					char el_type = task.buffer.consumeByte();
					std::string label = task.buffer.consumeString();
					unsigned int element_id = buttons_hash(label.c_str(), label.size());
					float col_hue = task.buffer.consumeFloat();
					float col_sat = task.buffer.consumeFloat();
					float col_bright = task.buffer.consumeFloat();

					switch(el_type) {
					case BTYPE_SLIDER: {
						int value_type = task.buffer.consumeByte();
						
						if(value_type == Slider<float>::SLIDER_FLOAT) {
							float value = task.buffer.consumeFloat();
							float minv = task.buffer.consumeFloat();
							float maxv = task.buffer.consumeFloat();
							float stepv = task.buffer.consumeFloat();
							float* dummy_value = new float;
							value_floats.push_back(dummy_value);							

							// CREATE FLOAT SLIDER
							Sliderf* sliderp = &gui->addFloat(label, *dummy_value).setMin(minv).setMax(maxv).setStep(stepv);
							sliderp->setValue(value);
							gui->setColor(col_hue);

							elements[buttons_id][element_id] = sliderp;
						}
						else if(value_type == Slider<float>::SLIDER_INT) {
							int value = task.buffer.consumeI32();
							int minv = task.buffer.consumeI32();
							int maxv = task.buffer.consumeI32();
							int stepv = task.buffer.consumeI32();
							int* dummy_value = new int;
							value_ints.push_back(dummy_value);

							Slideri* slideri = &gui->addInt(label, *dummy_value).setMin(minv).setMax(maxv).setStep(stepv);
							slideri->setValue(value);
							gui->setColor(col_hue);
							
							elements[buttons_id][element_id] = slideri;
						}
						break;
					}
					default: {
						printf("Error: Unhandled scheme type: %d\n", el_type);
						break;
					}}
				}
				break; // fror now we break after one el.
			}
			default:break;
			}
			break;
		}
	}

	// From Client > Server
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	void Client::onEvent(ButtonsEventType event, const Buttons& buttons, const Element* target) {
		if(event == BEVENT_VALUE_CHANGED) {
			ButtonsBuffer buffer;
			if(util.serialize(buttons, target, buffer)) {
				CommandData data;
				data.name = BDATA_CHANGED;
				data.buffer.addByte(data.name);
				data.buffer.addUI32(buffer.getNumBytes());
				data.buffer.addBytes(buffer.getPtr(), buffer.getNumBytes());
				addOutCommand(data);
			}
		}
	}

} // buttons