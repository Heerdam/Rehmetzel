#pragma once

#include <GL/glew.h>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

#include <glm/ext/vector_uint2.hpp>
#include <glm/ext/vector_uint3.hpp>
#include <glm/ext/vector_uint4.hpp>

#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_int3.hpp>
#include <glm/ext/vector_int4.hpp>

#include <glm/ext/matrix_float4x4.hpp>

#include <glm/gtx/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <omp.h>

#include <unordered_map>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>
#include <functional>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <limits>
#include <random>
#include <sstream>
#include <tuple>
#include <ctime>
#include <chrono>
#include <stdexcept>
#include <exception>
#include <typeinfo>
#include <regex>
#include <windows.h>
#include <stack>
#include <forward_list>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/types.h>

namespace Heerbann {

	using namespace Heerbann;

typedef unsigned int uint;
typedef unsigned short ushort;

typedef glm::ivec2 Vec2i;
typedef glm::ivec3 Vec3i;
typedef glm::ivec4 Vec4i;

typedef glm::uvec2 Vec2u;
typedef glm::uvec3 Vec3u;
typedef glm::uvec4 Vec4u;

typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;

#define UVX Vec4(1.f, 0.f, 0.f, 1.f)
#define UVY Vec4(0.f, 1.f, 0.f, 1.f)
#define UVZ Vec4(0.f, 0.f, 1.f, 1.f)

typedef glm::mat3 Mat3;
typedef glm::mat4 Mat4;

#define M00(X) (X[0][0])
#define M01(X) (X[0][1])
#define M02(X) (X[0][2])
#define M03(X) (X[0][3])

#define M10(X) (X[1][0])
#define M11(X) (X[1][1])
#define M12(X) (X[1][2])
#define M13(X) (X[1][3])

#define M20(X) (X[2][0])
#define M21(X) (X[2][1])
#define M22(X) (X[2][2])
#define M23(X) (X[2][3])

#define M30(X) (X[3][0])
#define M31(X) (X[3][1])
#define M32(X) (X[3][2])
#define M33(X) (X[3][3])

typedef glm::quat Quat;

#define CRS(X, Y) (glm::cross((X), (Y)))
#define DOT(X, Y) (glm::dot((X), (Y)))
#define DIS(X, Y) (glm::distance((X), (Y)))
#define LEN(X) (glm::length((X)))
#define NOR(X) (glm::normalize((X)))

#define DET(X) (glm::determinant((X)))
#define INV(X) (glm::inverse((X)))
#define TR(X) (glm::transpose((X)))
#define ToArray(X) (glm::value_ptr((X)))

/*returns an identity 4x4 matrix*/
#define IDENTITY glm::identity<Mat4>()

/*Creates a frustum matrix.*/
#define FRUSTUM(LEFT, RIGHT, BOTTOM, TOP, NEARPLANE, FARPLANE) (gml::frustm((LEFT), (RIGHT), (BOTTOM), (TOP), (NEARPLANE), (FARPLANE)))

/*Creates a matrix for a symmetric perspective-view frustum with far plane at infinite.
	FOVY: Expressed in radians if GLM_FORCE_RADIANS is define or degrees otherwise*/
#define INFINITEPERSPECTIVE(FOVY, ASPECT, NEARPLANE) (gml::infinitePerspective((FOVY), (ASPECT), (NEARPLANE)))

/*Build a look at view matrix.
	EYE: Position of the camera 
	CENTER: Position where the camera is looking at
	UP: Normalized up vector, how the camera is oriented. Typically (0, 0, 1)*/
#define LOOKAT(EYE, CENTER, UP) (glm::lookAt((EYE), (CENTER), (UP)))

/*Creates a matrix for an orthographic parallel viewing volume.*/
#define ORTHO(LEFT, RIGHT, BOTTOM, TOP, ZNEAR, ZFAR) (glm::ortho((LEFT), (RIGHT), (BOTTOM), (TOP), (ZNEAR), (ZFAR)))

/*Creates a matrix for projecting two-dimensional coordinates onto the screen.*/
#define ORTHO2D(LEFT, RIGHT, BOTTOM, TOP) (glm::ortho((LEFT), (RIGHT), (BOTTOM), (TOP)))

/*Creates a matrix for a symetric perspective-view frustum.
	FOVY: Expressed in radians if GLM_FORCE_RADIANS is define or degrees otherwise*/
#define PERSPECTIVE(FOVY, ASPECT, NEARPLANE, FARPLANE) (glm::perspective((FOVY), (ASPECT), (NEARPLANE), (FARPLANE)))

/*Builds a perspective projection matrix based on a field of view.
	FOVY: Expressed in radians if GLM_FORCE_RADIANS is define or degrees otherwise*/
#define PERSPECTIVEFOV(FOV, WIDTH, HEIGHT, NEARPLANE, FARPLANE) glm::perspectiveFov((FOV), (WIDTH), (HEIGHT), (NEARPLANE), (FARPLANE))

/*Define a picking region.*/
#define PICKMATRIX(CENTER, DELTA, VIEWPORT) (glm::pickMatrix((CENTER), (DELTA), (VIEWPORT))

/*Map the specified object coordinates (OBJECT.x, OBJECT.y, OBJECT.z) into window coordinates.*/
#define PROJECT(OBJECT, MODEL, PROJ, VIEWPORT) (glm::project((OBJECT), (MODEL), (PROJ), (VIEWPORT)))

/*Map the specified window coordinates (WINDOW.x, WINDOW.y, WINDOW.z) into object coordinates.*/
#define UNPROJECT(WINDOW, MODEL, PROJ, VIEWPORT) (glm::unProject((WINDOW), (MODEL), (PROJ), (VIEWPORT)))

/*Builds a rotation 4 * 4 matrix created from an axis vector and an angle.
	INPUTMATRIX: Input matrix multiplied by this rotation matrix.
	ANGLE: Rotation angle expressed in radians if GLM_FORCE_RADIANS is define or degrees otherwise.
	AXIS: Rotation axis, recommanded to be normalized.*/
#define ROTATEMATRIX(INPUTMATRIX, ANGLE, AXIS) (glm::rotate((INPUTMATRIX), (ANGLE), (AXIS)))

/*Builds a scale 4 * 4 matrix created from 3 scalars.
	INPUTMATRIX: Input matrix multiplied by this scale matrix.
	SCALERATIO: Ratio of scaling for each axis.
*/
#define SCALEMATRIX(INPUTMATRIX, SCALERATIO) (glm::scale((INPUTMATRIX), (SCALERATIO)))

/*Builds a translation 4 * 4 matrix created from a vector of 3 components.
	INPUTMATRIX: Input matrix multiplied by this translation matrix.
	TRANSLATIONVECTOR: Coordinates of a translation vector.*/
#define TRANSLATIONMATRIX(INPUTMATRIX, TRANSLATIONVECTOR) (glm::translate((INPUTMATRIX), (TRANSLATIONVECTOR)))

#define TOMAT4(X) (glm::toMat4((X)))
#define EULER(X) (glm::eulerAngles((X)))
#define ROLL(X) (glm::roll((X)))
#define YAW(X) (glm::yaw((X))))
#define PITCH(X) (glm::pitch((X)))

#define PI 3.14159265358979323846f
#define DEGTORAD static_cast<float>(PI / 180.0)
#define TORAD(X) ((X) * DEGTORAD)
#define RADTODEG static_cast<float>(180.0 / PI)
#define TODEG(X) ((X) * RADTODEG)
#define EQUAL(X, Y) (Heerbann::App::Util::almost_equal((X), (Y)))
#define ISNULL(X) (Heerbann::App::Util::almost_equal((X), (0.f)))
#define ABS(X) (std::fabsf(X))
#define CLAMP(X, MIN, MAX) (std::clamp((X), (MIN), (MAX)))
#define SIN(X) (std::sinf(X))
#define ASIN(X) (std::asinf(X))
#define COS(X) (std::cosf(X))
#define ACOS(X) (std::acos(X))
#define TAN(X) (std::tanf(X))
#define ATAN(X) (std::atan(X))
#define INF (std::numeric_limits<float>::infinity())
#define SQRT(X) (std::sqrtf((X)))
#define POW(X, Y) (std::powf((X), (Y)))

#define colF(X) ((X)/255.f)
	// 1 meter (box2d) is more or less 64 pixels (sfml)
#define RATIO 30.0f
#define PIXELS_PER_METER RATIO

// 64 pixels are more or less 1 meter (box2d)
#define UNRATIO (1.0F/RATIO)
#define METERS_PER_PIXEL UNRATIO

#define M_WIDTH Heerbann::App::Get()->width()
#define M_HEIGHT Heerbann::App::Get()->height()
#define M_ID Heerbann::App::Util::getId()
#define M_FloatBits(X) (Heerbann::App::Util::toFloatBits((X)))
#define M_s2ws(X) (Heerbann::App::Util::s2ws((X)))

#define M_Main Heerbann::App::Get()
#define M_Asset Heerbann::App::Get()->getAssetManager()
#define M_Input Heerbann::App::Get()->getInput()
#define M_Batch Heerbann::App::Get()->getBatch()
#define M_Font Heerbann::App::Get()->getFontCache()
#define M_Stage Heerbann::App::Get()->getStage()
#define M_Random Heerbann::App::Get()->getRandom()
#define M_View Heerbann::App::Get()->getViewport()
#define M_World Heerbann::App::Get()->getWorld()
#define M_Level Heerbann::App::Get()->getLevel()
#define M_Context Heerbann::App::Get()->getContext()
#define M_Timer Heerbann::App::Get()->getTimer()
#define M_Logger Heerbann::App::Get()->getLogger()
#define M_Shape Heerbann::App::Get()->getShape()
#define M_Env Heerbann::App::Get()->getEnv()

#define ID Heerbann::App::Util::getId()
#define DeltaTime Heerbann::App::Get()->deltaTime()
#define DefaultFont Heerbann::App::Get()->getDefaultFont()

#define TIMESTAMP Heerbann::App::Get()->getTimer()->timeStamp()
#define TIME_START Heerbann::App::Get()->getTimer()->start()
#define TIME_END Heerbann::App::Get()->getTimer()->end()

#define LOG(X) (Heerbann::App::Get()->getLogger()->log(X));

#define LERP(START, END, T) ((START) + ((END) - (START)) * (T))

#define GLError(X) (Heerbann::App::Gdx::printOpenGlErrors((X)))
#define FBError(X) (Heerbann::App::Gdx::printFrameBufferErrors((X)))

#define FLOAT(X) (static_cast<float>((X)))

using namespace std::chrono_literals;

	namespace App {
		class Main;
	}

	//Utils
	class SpriteBatch;
	class DebugDraw;
	class ShapeRenderer;

	//Math
	struct Ray;
	struct Plane;
	struct BoundingBox;
	struct Frustum;
	struct AABBTreeNode;
	class AABBTree;

	//UI
	namespace UI {
		class Stage;
		class Actor;
		class Label;	
	}

	//TextUtil
	namespace Text {
		class TextBlock;
		class FontCache;
		struct Line;
		struct Letter;
		enum Align : int;
		enum TextDecoration : int;
		class SplitFunctor;
	}

	//Level
	enum State : int;
	struct Level;
	struct PreLoadLevel;
	struct LoadingScreenLevel;
	struct MainMenuLevel;
	struct TestWorldLevel;
	class LevelManager;

	//Input
	struct InputEntry;
	class InputMultiplexer;

	//MapGenerator
	class MapGenerator;

	//Assets
	enum Type : int;
	struct Ressource;
	class AssetManager;
	class Image;
	class Texture2D;
	class Array2DTexture;
	struct AtlasRegion;
	class TextureAtlas;
	class SSBO;
	class FlipFlopSSBO;
	class Model;
	class ShaderProgram;
	class Framebuffer;
	class ShadowMap;
	class Font;
	class Renderer;
	struct TxDbgRenderable;
	class TextureDebugRenderer;
	struct ShadowRenderable;
	class ShadowRenderer;
	struct GaussianBlurRenderable;
	class GaussianBlurRenderer;
	struct VoxelRenderable;
	class VoxelBackGroundRenderer;
	struct VSMLightRenderable;
	class VSMLightRenderer;
	struct VSMShadownRenderable;
	class VSMShadowRenderer;
	struct VSMRenderable;
	class VSMRenderer;

	//World
	class VoxelWorld;
	struct WorldBuilderDefinition;

	//CameraUtils
	enum ViewType : int;
	class ViewportHandler;
	struct View;
	class Camera;
	class OrthographicCamera;
	class PerspectiveCamera;
	class AxisWidgetCamera;
	class OrthoPersCamera;

	//G3D
	enum AnimBehaviour : int;
	struct mNode;
	struct Bone;
	struct NodeAnimation;
	struct MeshKey;
	struct VectorKey;
	struct QuatKey;
	struct MeshAnimation;
	struct Animation;
	struct Mesh;
	struct ModelData;
	struct DrawCall;

	//Gdx
	class Environment;
	struct Light;
	struct PointLight;
	struct SpotLight;
	struct DirectionalLight;
	struct Material;
	enum LightType : int;
	struct sLight;


	//TimeLog
	class Logger;
	struct TimeStamp;
	class Timer;

	struct MainConfig {
		unsigned int MAXSPRITES = 1000;
		std::string name = "Unnamed";
		unsigned int windowWidth = 640;
		unsigned int windowHeight = 480;
		int windowStyle = sf::Style::Default;
		sf::ContextSettings settings;
	};

	namespace App {

		class Main {
		private:
			sf::RenderWindow* context;
			InputMultiplexer* inputListener;
			VoxelWorld* world;
			//ViewportHandler* viewports;
			AssetManager* assets;
			UI::Stage* stage;
			LevelManager* level;
			SpriteBatch* batch;
			Text::FontCache* cache;
			sf::Font* defaultFont; 
			ViewportHandler* viewport;
			Logger* logger;
			Timer* timer;
			ShapeRenderer* shape;
			Environment* env;

			Main();

			static Main* instance;

			std::mt19937_64 random;

			std::queue<std::tuple<std::function<bool(void*)>, void*>> loadJob;
			std::mutex jobLock;

			GLuint* indexBuffer;

		public:

			unsigned long long frameId = 1;

			~Main();

			void update();
			void intialize(MainConfig*);

			static Main* getInstance();

			static GLuint* getIndexBuffer();

			float deltaTime();

			//---------------------- AI ----------------------\\

			//static AI::AIHandler* getAI();

			//---------------------- Font ----------------------\\

			static Text::FontCache* getFontCache();
			static sf::Font* getDefaultFont();
			static void intializeFont(sf::Font*);

			//---------------------- Job ----------------------\\

			//thread safe
			static void addJob(std::function<bool(void*)>, void*);

			//---------------------- Random ----------------------\\

			static void setSeed(long);
			//random in interval [0, 1]
			static float getRandom();
			static float getRandom(float, float);

			//---------------------- Context ----------------------\\

			static sf::RenderWindow* getContext();
			static void setSize(unsigned int _width, unsigned int _height);

			inline static unsigned int width() {
				return getInstance()->getContext()->getSize().x;
			};

			inline static unsigned int height() {
				return getInstance()->getContext()->getSize().y;
			};

			static ShapeRenderer* getShape();

			//---------------------- TIME & Logger ----------------------\\

			static Timer* getTimer();
			static Logger* getLogger();

			//---------------------- Batch ----------------------\\

			static SpriteBatch* getBatch();

			//---------------------- Inputs ----------------------\\

			static InputMultiplexer* getInput();

			//---------------------- World ----------------------\\

			static VoxelWorld* getWorld();
			static Environment* getEnv();
		
			//---------------------- Viewport ----------------------\\

			static ViewportHandler * getViewport();

			//---------------------- Assets ----------------------\\

			static AssetManager* getAssetManager();

			//---------------------- Stage ----------------------\\

			static UI::Stage* getStage();

			//---------------------- Level ----------------------\\

			static LevelManager* getLevel();
		};

		inline Main* Get() {
			return Main::getInstance();
		}

		namespace Gdx {

			inline void printOpenGlErrors(std::string _id) {
				bool hasError = false;
				GLenum err;
				while ((err = glGetError()) != GL_NO_ERROR) {
					if (!hasError) {
						hasError = true;
						std::cout << "---- Print OpenGl Errors: " << _id << " ----" << std::endl;
					}
					if (err == GL_INVALID_VALUE)
						std::cout << "Invalid Value"; 
					else if(err == GL_INVALID_OPERATION)
						std::cout << "Invalid Operation";
					else if (err == GL_OUT_OF_MEMORY)
						std::cout << "Out of Memory";
					else if (err == GL_INVALID_ENUM)
						std::cout << "Invalid Enum";
					else if (err == GL_STACK_OVERFLOW)
						std::cout << "Stack Overflow";
					else if (err == GL_STACK_UNDERFLOW)
						std::cout << "Stack Underflow";
					else if (err == GL_INVALID_FRAMEBUFFER_OPERATION)
						std::cout << "Invalid Framebuffer Operation";
					else if (err == GL_CONTEXT_LOST)
						std::cout << "Context Lost";

					std::cout << " (" << err << ")" << std::endl;
				}
				if (hasError) std::cout << "---- Finished ----" << std::endl;
			};

			inline void printFrameBufferErrors(std::string _id) {
				switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
				case GL_FRAMEBUFFER_COMPLETE:
					return;
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
					std::cout << "---- Print Framebuffer Errors: " << _id << " ----" << std::endl;
					std::cout << "FRAMEBUFFER INCOMPLETE ATTACHMENT" << std::endl;
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
					std::cout << "---- Print Framebuffer Errors: " << _id << " ----" << std::endl;
					std::cout << "FRAMEBUFFER INCOMPLETE MISSING ATTACHMENT" << std::endl;
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
					std::cout << "---- Print Framebuffer Errors: " << _id << " ----" << std::endl;
					std::cout << "FRAMEBUFFER INCOMPLETE DRAW BUFFER" << std::endl;
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
					std::cout << "---- Print Framebuffer Errors: " << _id << " ----" << std::endl;
					std::cout << "FRAMEBUFFER INCOMPLETE READ BUFFER" << std::endl;
					break;
				case GL_FRAMEBUFFER_UNSUPPORTED:
					std::cout << "---- Print Framebuffer Errors: " << _id << " ----" << std::endl;
					std::cout << "FRAMEBUFFER UNSUPPORTED" << std::endl;
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
					std::cout << "---- Print Framebuffer Errors: " << _id << " ----" << std::endl;
					std::cout << "FRAMEBUFFER INCOMPLETE MULTISAMPLE" << std::endl;
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
					std::cout << "---- Print Framebuffer Errors: " << _id << " ----" << std::endl;
					std::cout << "FRAMEBUFFER INCOMPLETE LAYER TARGETS" << std::endl;
					break;
				}
				std::cout << "---- Finished ----" << std::endl;
			};

			inline unsigned long long getFrameId() {
				return App::Get()->frameId;
			};

		}

		namespace Util {

			long long inline getId() {
				static long id = 1000;
				return ++id;
			}

			std::wstring s2ws(const char*);
			std::wstring s2ws(const std::string&);
			std::vector<std::wstring> split(std::wstring, std::wstring);

			float toFloatBits(int _r, int _g, int _b, int _a);

			inline float toFloatBits(sf::Color _color) {
				return toFloatBits(_color.r, _color.g, _color.b, _color.a);
			};

			inline bool almost_equal(float _x, float _y) {
				return std::abs(_x - _y) <= std::numeric_limits<float>::epsilon() * (std::abs(_x) + (std::abs(_y) + 1.0f));
			};
			
			//#define ipc(X) *((int*)&X)
			//bool almost_equal(float a, float b){
				//return !((ipc(a) ^ ipc(b)) & (0xfffff000));
			//}
			

		}

	}

}


