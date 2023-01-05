#include <alsa/asoundlib.h>
#include <cmath>
#include <string>
#include <vector>
#include <cmath>
#include <istream>
#include <fstream>
#include <sstream>
#include "Engine.h"
#include <pulse/simple.h>
#include <list>

/**
 * @return module of result
 */
std::vector<double> FFT(double* fft, int n) {
	std::vector<double> amp;
	double dataFr[n * 2];
	double dataFi[n * 2];
	double dataR[n];
	double dataI[n];
	unsigned int k;
	int i, j;
	int it, m, is, nv;
	double p, q, s, vr, vi, poddr, poddi;

	// init
	k = log10(n) / log10(2);
	for (i = 0; i < n; ++i) {
		dataR[i] = *(fft + i);
		dataI[i] = 0;
	}

	// out
	for (it = 0 ; it <= n - 1 ; ++it) {
		m = it;
		is = 0;
		for (i = 0 ; i <= k - 1 ; ++i) {
			j = m / 2;
			is = 2 * is + (m - 2 * j);
			m = j;
		}
		dataFr[it] = dataR[is];
		dataFi[it] = dataI[is];
	}

	// ==

	dataR[0] = 1.0;
	dataI[0] = 1.0;
	p = 6.283185306/(1.0*n);
	dataR[1] = cos(p);
	dataI[1] = -sin(p);

	// calculate dataR and dataI

	for (i = 2 ; i <= n - 1 ; ++i) {
		p = dataR[i - 1] * dataR[1];
		q = dataI[i - 1] * dataI[1];
		s = (dataR[i - 1] + dataI[i - 1]) * (dataR[1] + dataI[1]);
		dataR[i] = p - q;
		dataI[i] = s - p - q;
	}

	// calculate fr and fi

	for (it = 0 ; it <= n - 2 ; it += 2) {
		vr = dataFr[it];
		vi = dataFi[it];
		dataFr[it] = vr + dataFr[it + 1];
		dataFi[it] = vi + dataFi[it + 1];
		dataFr[it + 1] = vr - dataFr[it + 1];
		dataFi[it + 1] = vi - dataFi[it + 1];
	}

	// but op

	m = n / 2;
	nv = 2;
	for (i = k - 2 ; i >= 0 ; --i) {
		m /= 2;
		nv *= 2;
		for (it = 0 ; it < (m - 1) * nv ; it += nv) {
			for (j = 0 ; j <= (nv / 2) - 1 ; ++j) {
				p = dataR[m * j] * dataFr[it + j + nv / 2];
				q = dataI[m * j] * dataFi[it + j + nv / 2];
				s = dataR[m * j] + dataI[m * j];
				s *= dataFr[it + j + nv / 2] + dataFi[it + j + nv / 2];
				poddr = p - q;
				poddi = s - p - q;
				dataFr[it + j + nv / 2] = dataFr[it + j] - poddr;
				dataFi[it + j + nv / 2] = dataFi[it + j] - poddi;
				dataFr[it + j] += poddr;
				dataFi[it + j] += poddi;
			}
		}
	}

	//calculate amp

	for (i = 0 ; i <= n - 1 ; ++i) {
//		amp.emplace_back(sqrt(dataFr[i] * dataFr[i] + dataFi[i] * dataFi[i]));
		amp.emplace_back(sqrt(dataFr[i] * dataFr[i] + dataFi[i] * dataFi[i]) / 48000 * 2);
	}
	return amp;
}

inline static short byteArrayToShortLE(const char* b, int offset) {
	short value = 0;
	for (int i = 0; i < 2; ++i) {
		value |= (((short)(b[i + offset])) & 0x000000FF) << (i * 8);
	}
	return value;
}

const double log10_100 = log10(100);

inline double log100(double tar) {
	return log10(tar) / log10_100;
}

void Engine::loop(GLFWwindow* window) {

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	std::string vertexCode, fragmentCode;
	std::ifstream vShaderFile, fShaderFile;
	vShaderFile.exceptions(std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::badbit);
	vShaderFile.open("./vertex.vsh");
	fShaderFile.open("./fragment.fsh");
	std::stringstream vShaderStream, fShaderStream;
	vShaderStream << vShaderFile.rdbuf();
	fShaderStream << fShaderFile.rdbuf();
	vShaderFile.close();
	fShaderFile.close();
	vertexCode = vShaderStream.str();
	fragmentCode = fShaderStream.str();

	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar* fShaderCode = fragmentCode.c_str();

	int succ;
	char infoLog[512];

	unsigned int vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vShaderCode, nullptr);
	glCompileShader(vertexShaderID);

	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &succ);
	glGetShaderInfoLog(vertexShaderID, 512, nullptr, infoLog);
	printf("%d => %s", succ, infoLog);


	unsigned int fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fShaderCode, nullptr);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &succ);
	glGetShaderInfoLog(fragmentShaderID, 512, nullptr, infoLog);
	printf("%d => %s\n", succ, infoLog);

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShaderID);
	glAttachShader(shaderProgram, fragmentShaderID);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &succ);
	glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
	printf("%d => %s\n", succ, infoLog);

	int bufSize = 8192;
	char buffer[bufSize];

	static const pa_sample_spec ss = {
			.format = PA_SAMPLE_S16LE,
			.rate = 48000,
			.channels = 2
	};

	pa_simple* s;
	int error;

	pa_mainloop* mainloop = pa_mainloop_new();
	pa_mainloop_api* mainAPI = pa_mainloop_get_api(mainloop);
	pa_context* context = pa_context_new(mainAPI, "AudioFFT");

	PulseAudioContextState state = INIT;
	pa_context_set_state_callback(context, stateCallback, &state);
	pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
	while (state == INIT) {
		pa_mainloop_iterate(mainloop, 1 ,nullptr);
	}

	std::list<Device> sources;
	pa_operation* op = pa_context_get_source_info_list(context, &sourceListCallback, &sources);
	while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
		pa_mainloop_iterate(mainloop, 1, nullptr);
	}
	pa_operation_unref(op);
	pa_context_set_state_callback(context, nullptr, nullptr);
	pa_context_unref(context);
	pa_mainloop_free(mainloop);
	printf("Select a Device: \n");
	for (int i = 0 ; i < used.size() ; ++i) {
		printf("%d: %s\n\t%s\n", i, used[i].name.c_str(), used[i].desc.c_str());
	}
	int dev;
	scanf("%d", &dev);
	s = pa_simple_new(nullptr, "AudioFFT", PA_STREAM_RECORD, used[dev].name.c_str(), "record", &ss, nullptr, nullptr, &error);

	printf("PulseAudio Simple API initialized, using device %s\n", used[dev].name.c_str());

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	//glEnable(GL_MULTISAMPLE);
	const float smoothConstantDown = 0.2;
	const float smoothConstantUp = 0.8;

	float lastHigh[4096] = {-1};

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		pa_simple_read(s, buffer, sizeof(buffer), &error);
		double fft[4096];
		for (int j = 0; j < 4096; ++j) {
			fft[j] = byteArrayToShortLE(buffer, j * 2);
		}

		for (int i = 0; i < 4096; ++i) {
			fft[i] = (0.5 * (1.0 - cos(2.0 * acos(-1) * i / 4095))) * fft[i];
		}

		std::vector<double> out = FFT(fft, 4096);

		for (int i = 0; i < out.size(); ++i) {
			if (out[i] < lastHigh[i]) {
				lastHigh[i] = out[i] * smoothConstantDown + lastHigh[i] * (1 - smoothConstantDown);
			} else {
				lastHigh[i] = out[i] * smoothConstantUp + lastHigh[i] * (1 - smoothConstantUp);
			}
		}

		float vertices[3600];

		for (int rects = 1 ; rects <= 200 ; ++rects) {
			float high = (((float)(lastHigh[rects-1]))) - 1.0;
			float left = (log100(rects)) / 1.1515989021 * 2 - 1;
			float right = log100(rects + 1)  / 1.1515989021 * 2 - 1;
			for (int tri = 0 ; tri < 2 ; ++tri) {
				if (tri == 0) {
					vertices[((rects-1) * 2 + tri) * 9] = right;
					vertices[((rects-1) * 2 + tri) * 9 + 1] = high;
					vertices[((rects-1) * 2 + tri) * 9 + 2] = 0.0;

					vertices[((rects-1) * 2 + tri) * 9 + 3] = left;
					vertices[((rects-1) * 2 + tri) * 9 + 4] = high;
					vertices[((rects-1) * 2 + tri) * 9 + 5] = 0.0;

					vertices[((rects-1) * 2 + tri) * 9 + 6] = left;
					vertices[((rects-1) * 2 + tri) * 9 + 7] = -1.0;
					vertices[((rects-1) * 2 + tri) * 9 + 8] = 0.0;
				} else {
					vertices[((rects-1) * 2 + tri) * 9] = right;
					vertices[((rects-1) * 2 + tri) * 9 + 1] = high;
					vertices[((rects-1) * 2 + tri) * 9 + 2] = 0.0;

					vertices[((rects -1)* 2 + tri) * 9 + 3] = left;
					vertices[((rects-1) * 2 + tri) * 9 + 4] = -1.0;
					vertices[((rects-1) * 2 + tri) * 9 + 5] = 0.0;

					vertices[((rects-1) * 2 + tri) * 9 + 6] = right;
					vertices[((rects -1)* 2 + tri) * 9 + 7] = -1.0;
					vertices[((rects-1) * 2 + tri) * 9 + 8] = 0.0;
				}
			}
		}

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3600);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
