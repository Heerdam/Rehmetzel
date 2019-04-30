#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>

#include <Box2D/Box2D.h>

namespace Heerbann {

	using namespace Heerbann;

//diameter of a cell of the background
//#define BG_CELLDIAMETER 100

//how many cells a background VAO holds
//#define BG_CELLCOUNT 50

//#define SMALLFONTSIZE 18
//#define MEDIUMFONTSIZE 24
//#define BIGFONTSIZE 50

//how many sprites the spritebatch holds
//#define TEXTURECOUNT 15

#define VERTEXSIZE 8 //pos + index + typ + uv + color1 + color2
#define TYP_SPRITE 0.f
#define TYP_FONT 1.f
#define TYP_LINE 2.f
#define TYP_BOX 3.f
#define TYP_FONT_STATIC 4.f

typedef unsigned int uint;

typedef glm::ivec2 Vec2i;
typedef glm::ivec3 Vec3i;
typedef glm::ivec4 Vec4i;

typedef glm::uvec2 Vec2ui;
typedef glm::uvec3 Vec3ui;
typedef glm::uvec4 Vec4ui;

typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;

#define UVX Vec3(1.f, 0.f, 0.f)
#define UVY Vec3(0.f, 1.f, 0.f)
#define UVZ Vec3(0.f, 0.f, 1.f)

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

#define PI b2_pi
#define DEGTORAD static_cast<float>(PI / 180.0)
#define TORAD(X) ((X) * DEGTORAD)
#define RADTODEG static_cast<float>(180.0 / PI)
#define TODEG(X) ((X) * RADTODEG)
#define EQUAL(X, Y) (Heerbann::App::Util::almost_equal((X), (Y)))
#define ISNULL(X) (Heerbann::App::Util::almost_equal((X), (0.f)))
#define ABS(X) (std::fabsf(X))
#define CLAMP(X, MIN, MAX) (std::clamp(X, MIN, MAX))
#define SIN(X) (std::sinf(X))
#define ASIN(X) (std::asinf(X))
#define COS(X) (std::cosf(X))
#define ACOS(X) (std::acos(X))
#define TAN(X) (std::tanf(X))
#define ATAN(X) (std::atan(X))
#define INF (std::numeric_limits<float>::infinity())
#define SQRT(X) (std::sqrtf((X)))

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

#define DeltaTime Heerbann::App::Get()->deltaTime()

#define TIMESTAMP Heerbann::App::Get()->getTimer()->timeStamp()
#define TIME_START Heerbann::App::Get()->getTimer()->start()
#define TIME_END Heerbann::App::Get()->getTimer()->end()

#define LOG(X) (Heerbann::App::Get()->getLogger((X)));

#define LERP(START, END, T) ((START) + ((END) - (START)) * (T))


	namespace App {
		class Main;
	}

	//Utils
	class VAO;
	class BGVAO;
	class IndexedVAO;
	class SpriteBatch;
	class ShaderProgram;
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
		class StaticLabel;		
	}

	//TextUtil
	namespace Text {
		class TextBlock;
		class FontCache;
		struct StaticTextBlock;
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

	//G3D
	namespace G3D {
		struct Model;
	}

	//Assets
	enum Type : int;
	enum State : int;
	struct LoadItem;
	class AssetManager;
	struct TextureAtlas;
	struct AtlasRegion;
	class TextureAtlasLoader;

	//World
	enum EntityType : int;
	struct WorldObject;
	class World;
	struct WorldBuilderDefinition;
	struct WorldOut;
	class WorldBuilder;

	//CameraUtils
	enum ViewType : int;
	class ViewportHandler;
	class View;
	class Camera;
	class OrthographicCamera;
	class PerspectiveCamera;
	class AxisWidgetCamera;

	//Gdx
	class Environment;
	struct Light;
	struct PointLight;
	struct SpotLight;
	struct DirectionalLight;
	struct Material;
	enum DrawableType : int;
	struct Drawable;

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
			World* world;
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

			Main();

			static Main* instance;

			std::mt19937_64 random;

			std::queue<std::tuple<std::function<void(void*)>, void*>> loadJob;
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
			static void addJob(std::function<void(void*)>, void*);

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

			//---------------------- TIME & Logger ----------------------\\

			static Timer* getTimer();
			static Logger* getLogger();

			//---------------------- Batch ----------------------\\

			static SpriteBatch* getBatch();

			//---------------------- Inputs ----------------------\\

			static InputMultiplexer* getInput();

			//---------------------- World ----------------------\\

			static World* getWorld();

		
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
					std::cout << err << std::endl;
				}
				if (hasError) std::cout << "---- Finished ----" << std::endl;
			};

			inline unsigned long getFrameId() {
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

			constexpr bool almost_equal(float _x, float _y) {
				return std::abs(_x - _y) <= std::numeric_limits<float>::epsilon() * (std::abs(_x) + (std::abs(_y) + 1.0f));
			};

		}

	}

}


