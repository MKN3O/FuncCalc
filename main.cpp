// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp
#include "ImNodeFlow.h"
#include <algorithm>
#include "implot.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <vector>
#include <fstream>
#include <tinyfiledialogs.h>
#pragma warning (disable: 4996)
#pragma once


namespace styles {
	auto errorColor = std::make_shared<ImFlow::NodeStyle>(IM_COL32(255, 0, 0, 255), ImColor(255, 255, 255), 6.5f);
	auto fileColor = std::make_shared<ImFlow::NodeStyle>(IM_COL32(47, 235, 219, 255), ImColor(0, 0, 0), 6.5f);
	auto workColor = std::make_shared<ImFlow::NodeStyle>(IM_COL32(0, 0, 0, 255), ImColor(255, 255, 255, 255), 6.5f);
	auto graphColor = std::make_shared<ImFlow::NodeStyle>(IM_COL32(240, 224, 10, 255), ImColor(0, 0, 0), 6.5f);
};


std::vector<double> ReadColumn(std::string fname, char spf) {
	std::vector<double> v{};
	int k = 0;
	std::string s{}, buffer{};
	std::ifstream fileStrmIn;
	bool error = 0;
	try {
		fileStrmIn = std::ifstream(fname);
	}
	catch (...) {
		error = 1;
	}
	if (!error) {
		try {
			while (std::getline(fileStrmIn, s), s != "") {
				if (spf == 'x') {
					for (char c : s) {
						if (c == ' ') {
							break;
						}
						buffer += c;
					}
					v.push_back(std::stod(buffer));
					buffer = "";
				}
				if (spf == 'y') {
					for (char c : s) {
						if (c == ' ') {
							k += 1;
						}
						if (k == 2) {
							break;
						}
						if (k >= 1) {
							buffer += c;
						}
					}
					v.push_back(std::stod(buffer));
					buffer = "";
					k = 0;
				}
				if (spf == 'z') {
					for (char c : s) {
						if (c == ' ') {
							k += 1;
						}
						if (k == 3) {
							break;
						}
						if (k >= 2) {
							buffer += c;
						}
					}
					v.push_back(std::stod(buffer));
					buffer = "";
					k = 0;
				}
				if (spf == 't') {
					for (char c : s) {
						if (c == ' ') {
							k += 1;
						}
						if (k >= 3) {
							buffer += c;
						}
					}
					v.push_back(std::stod(buffer));
					buffer = "";
					k = 0;
				}

			}
		}
		catch (...) {
			fileStrmIn.close();
			throw;
		}
		if (fileStrmIn) {
			fileStrmIn.close();
		}
	}
	return v;
}



class File {
public:
	std::vector<double> x{};
	std::vector<double> y{};
	std::vector<double> z{};
	std::vector<double> t{};
	std::string path{};
	File(std::string fname = "") {
		path = fname;
		x = ReadColumn(path, 'x');
		y = ReadColumn(path, 'y');
		z = ReadColumn(path, 'z');
		t = ReadColumn(path, 't');
	}
};


class FileNode : public ImFlow::BaseNode
{
public:
	FileNode(File file)
	{
		setTitle(file.path);
		setStyle(styles::fileColor);
		addOUT<std::vector <double>>("X")
			->behaviour([=]() { return file.x; });
		addOUT < std::vector <double >>("Y")
			->behaviour([=]() { return file.y; });
		addOUT<std::vector <double>>("Z")
			->behaviour([=]() { return file.z; });
		addOUT<std::vector <double>>("T")
			->behaviour([=]() { return file.t; });
	}
	void draw() override {
		if (ImGui::Button("Destroy the node")) {
			this->destroy();
		}
	}
};


class Plotting : public ImFlow::BaseNode {
private:
	std::shared_ptr<std::vector<double>> Yaxis;
	std::shared_ptr<std::vector<double>> Xaxis;
	std::shared_ptr<bool> deleteNode;
public:
	Plotting() {
		setTitle("Plotter");
	}
	Plotting(std::shared_ptr<std::vector<double>> XAxis, std::shared_ptr<std::vector<double>> YAxis, int number, std::shared_ptr<bool> deletedNode) {
		deleteNode = deletedNode;
		Xaxis = XAxis;
		Yaxis = YAxis;
		std::string title = "Plotter" + std::to_string(number);
		setTitle(title);
		setStyle(ImFlow::NodeStyle::red());
	}
	void draw() override {
		setStyle(styles::graphColor);
		*Xaxis = showIN<std::vector<double>>("X axis Input", {}, ImFlow::ConnectionFilter::SameType());
		*Yaxis = showIN<std::vector<double>>("Y axis Input", {}, ImFlow::ConnectionFilter::SameType());
		if (Xaxis->size() != Yaxis->size() && Xaxis->size() != 0 && Yaxis->size() != 0) {
			ImGui::Text("Warning! Vectors have different size.\nPlotting by minimal size of two vectors.");
			setStyle(styles::errorColor);
		}
		else setStyle(styles::graphColor);
		if (ImGui::Button("Destroy the node")) {
			*deleteNode = 1;
			this->destroy();
		}
	}
};


class GraphingNode {
private:
	std::string title;
	static inline unsigned count{};
public:
	std::shared_ptr<std::vector<double>> Xaxis;
	std::shared_ptr<std::vector<double>> Yaxis;
	std::shared_ptr<bool> deletedNode = std::make_shared<bool>(nullptr);
	const char* titl;
public:
	GraphingNode(ImFlow::ImNodeFlow& grid) {
		++count;
		Xaxis = std::make_shared<std::vector<double>>();
		Yaxis = std::make_shared<std::vector<double>>();
		title = "Plotter" + std::to_string(count);
		*deletedNode = 0;
		grid.addNode<Plotting>({ 700, 30 }, Xaxis, Yaxis, count, deletedNode);
	}
	void draw() {
		titl = title.c_str();
		ImPlot::SetNextAxesLimits(0, 10, 0, 10);
		ImGui::SetNextWindowSize({ 450, 335 });
		ImGui::Begin(titl);
		if (ImPlot::BeginPlot(titl)) { //plotting
			ImPlot::SetupAxes("x", "y");
			ImPlot::PlotLine("Plot", (*Xaxis).data(), (*Yaxis).data(), std::min((*Xaxis).size(), (*Yaxis).size()));
			ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
			ImPlot::EndPlot();
		}
		ImGui::End();
	}
};


class SummationNode : public ImFlow::BaseNode
{
private:
	std::vector<double> FirstElem;
	std::vector<double> SecondElem;
	std::vector<double> SumElem;
	bool subtraction = 0;
public:
	SummationNode()
	{
		setTitle("SummationNode");
	}
	void draw() override
	{
		/*setStyle(styles::workColor);*/
		FirstElem = showIN<std::vector<double>>("First elem Input", {}, ImFlow::ConnectionFilter::SameType());
		SecondElem = showIN<std::vector<double>>("Second elem Input", {}, ImFlow::ConnectionFilter::SameType());
		if (FirstElem.size() != SecondElem.size() && FirstElem.size() != 0 && SecondElem.size() != 0) {
			ImGui::Text("Warning! Vectors have different size.\nSummation by minimal size of two vectors.");
			setStyle(styles::errorColor);
		}
		else setStyle(styles::workColor);
		SumElem = {};
		ImGui::Checkbox("Subtract vectors", &subtraction);
		if (subtraction) {
			for (int i = 0; i < std::min(FirstElem.size(), SecondElem.size()); i++) {
				SumElem.push_back(FirstElem[i] - SecondElem[i]);
			}
		}
		else {
			for (int i = 0; i < std::min(FirstElem.size(), SecondElem.size()); i++) {
				SumElem.push_back(FirstElem[i] + SecondElem[i]);
			}
		}
		showOUT<std::vector<double>>("OutSum", ([this]() {return SumElem; }));
		if (ImGui::Button("Destroy the node")) {
			this->destroy();
		}
	}
private:
	int m_valB = 0;
};


class MultiplicationNode : public ImFlow::BaseNode {
private:
	std::vector<double> FirstElem;
	std::vector<double> SecondElem;
	std::vector<double> MulpElem;
	double scalar = 1;
	bool ElemMultiply = 0;
	bool ScalarProduct = 0;
	double Product = 0;
public:
	MultiplicationNode()
	{
		setTitle("MultiplicationNode");
	}
	void draw() override {
		FirstElem = showIN<std::vector<double>>("First elem Input", {}, ImFlow::ConnectionFilter::SameType());
		MulpElem = {};
		ImGui::SetNextItemWidth(140);
		ImGui::InputDouble("MultiplyByScalar", &scalar, 0.1, 0.5);
		ImGui::Checkbox("ElemMultiply", &ElemMultiply);
		Product = 0;
		if (ElemMultiply) {
			ImGui::Checkbox("ScalarProduct", &ScalarProduct);
			SecondElem = showIN<std::vector<double>>("Second elem Input", {}, ImFlow::ConnectionFilter::SameType());
			if (FirstElem.size() != SecondElem.size() && FirstElem.size() != 0 && SecondElem.size() != 0) {
				ImGui::Text("Warning! Vectors have different size.\nMultiplication by minimal size of two vectors.");
				setStyle(styles::errorColor);
			}
			else setStyle(styles::workColor);
			if (ScalarProduct) {
				for (int i = 0; i < std::min(FirstElem.size(), SecondElem.size()); i++) {
					MulpElem.push_back(FirstElem[i] * SecondElem[i] * scalar);
					Product += FirstElem[i] * SecondElem[i];
				}
				ImGui::SetNextItemWidth(140);
				ImGui::InputDouble("<-Scalar Product", &Product, 0, 0);
			}
			else
				for (int i = 0; i < std::min(FirstElem.size(), SecondElem.size()); i++) {
					MulpElem.push_back(FirstElem[i] * SecondElem[i] * scalar);
				}
		}
		else {
			for (int i = 0; i < FirstElem.size(); i++) {
				MulpElem.push_back(FirstElem[i] * scalar);
			}
		}
		showOUT<std::vector<double>>("OutMulp", ([this]() {return MulpElem; }));
	}
};


class FourierExpansion : public ImFlow::BaseNode {
	std::vector<double> x{};
	std::vector<double> y{};
	std::vector<double> y1{};
	double a0;
	double an;
	int n = 1;
	double sum = 0;
public:
	FourierExpansion() {
		setTitle("FourierExpansion");
		setStyle(ImFlow::NodeStyle::brown());
	}
	void draw() {
		a0 = 0;
		y1 = {};
		y1.resize(x.size());
		x = showIN<std::vector<double>>("X", { }, ImFlow::ConnectionFilter::SameType());
		y = showIN<std::vector<double>>("Y", { }, ImFlow::ConnectionFilter::SameType());
		ImGui::SetNextItemWidth(180);
		ImGui::SliderInt("N", &n, 1, 40);
		if (x.size() != 0 && y.size() != 0) {
			an = 0;
			for (int i = 0; i < std::min(x.size(), y.size()) - 1; i++) {
				a0 += 2 * (x[i + 1] - x[i]) * ((y[i + 1] + y[i]) / 2) / x[x.size() - 1];
			} // calculate a0
			for (int i = 0; i < std::min(x.size(), y.size()); i++) {
				y1[i] = a0 / 2;
				an = 0;
				for (int k = 1; k < n; k++) { // Fourier summ
					if (x[0] == 0) { // expansion by cosinuses
						for (int j = 0; j < std::min(x.size(), y.size()) - 1; j++) {
							an += (2.0 / x[x.size() - 1]) * (x[j + 1] - x[j]) * ((y[j + 1] * std::cos(3.14 * k * x[j + 1] / x[x.size() - 1]) + y[j] * std::cos(3.14 * k * x[j] / x[x.size() - 1])) / 2); //integrating an
						}
					}
					y1[i] += an * cos(3.14 * k * x[i] / x[x.size() - 1]);
					an = 0;
				}
			}
			for (int i = 0; i < y1.size(); i++) {
				y1[i] = abs(y[i] - y1[i]);
			}
		}

		showOUT<std::vector<double>>("XOut", ([this]() {return x; }));
		showOUT<std::vector<double>>("YOut", ([this]() {return y1; }));
		if (ImGui::Button("Destroy the node")) {
			this->destroy();
		}
	}
};


class IntegralNode : public ImFlow::BaseNode {
	std::vector<double> x{};
	std::vector<double> y{};
public:
	IntegralNode() {
		setTitle("IntegralFunc");
	}
	void draw() override {
		setStyle(styles::workColor);
		if (ImGui::Button("DelNode")) {
			this->destroy();
		}
		x = showIN <std::vector<double >>("X", { }, ImFlow::ConnectionFilter::SameType());
		y = showIN<std::vector<double>>("Y", { }, ImFlow::ConnectionFilter::SameType());
		double sum = 0;
		if ((x.size() != 0) && (y.size() != 0)) {
			for (int i = 0; i < std::min(x.size(), y.size()) - 1; i++) {
				sum += (x[i + 1] - x[i]) * ((y[i + 1] + y[i]) / 2);
			}
			ImGui::SetNextItemWidth(140);
			ImGui::InputDouble("<-Integral Sum", &sum, 0, 0);
		}
	}
};


static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


// Main code
int main()
{
	File* file;
	std::vector<GraphingNode> GraphVector = {};
	char* lTheOpenFileName = NULL;
	char const* lFilterPatterns[2] = { "*.txt", "*.text" };
	std::vector<File> filelist{};
	// Initialization of GLFW and window creation
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "I want to cry", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialization of ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Initialization of ImGui for GLFW and OpenGL
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImFlow::ImNodeFlow MyGrid; // Creating Grid
	// Main application loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		// Start a new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Create a window with a button
		ImGui::Begin("Main Window");
		ImGui::Text("Creating Nodes:");
		ImGui::SameLine();
		if (ImGui::Button("SumNode")) 	MyGrid.addNode<SummationNode>({ 400, 200 }); // adding nodes
		ImGui::SameLine();
		if (ImGui::Button("MultNode")) 	MyGrid.addNode<MultiplicationNode>({ 600, 200 });
		ImGui::SameLine();
		if (ImGui::Button("IntegralNode")) 	MyGrid.addNode<IntegralNode>({ 800, 200 });
		ImGui::SameLine();
		if (ImGui::Button("FourierExpansion")) 	MyGrid.addNode<FourierExpansion>({ 800, 200 });
		ImGui::SameLine();
		if (ImGui::Button("GraphNode")) GraphVector.emplace_back(GraphingNode(MyGrid));
		if (ImGui::Button("Open File")) { // opening files
			lTheOpenFileName = tinyfd_openFileDialog("let us read the file", "../", 2, lFilterPatterns, "text files", 1);
			if (lTheOpenFileName) {
				char* s = new char[strlen(lTheOpenFileName)];
				s = strtok(lTheOpenFileName, "|");
				try {
					File file(s);
					filelist.push_back(file);
					MyGrid.addNode<FileNode>({ 0, 200 }, file);
				}
				catch (...) {
					tinyfd_messageBox(
						"Error",
						"Your file have bad format!!!",
						"ok",
						"error",
						0);
				}
				while (s = strtok(NULL, "|")) {
					try {
						File file(s);
						filelist.push_back(file);
						MyGrid.addNode<FileNode>({ 0, 200 }, file);
					}
					catch (...) {
						tinyfd_messageBox(
							"Error",
							"Your file have bad format!!!",
							"ok",
							"error",
							0);
					}
				}
				delete[] s;
			}
		}
		for (int i = 0; i < GraphVector.size(); i++) {
			if (*(GraphVector[i].deletedNode)) {
				GraphVector.erase(std::next(GraphVector.begin(), i));
			}
			else
				GraphVector[i].draw();
		}
		ImGui::Begin("Work Area");
		MyGrid.update();
		ImGui::End();
		ImGui::End();
		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	// Cleanup ImGui resources
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	// Close the window and cleanup GLFW resources
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
