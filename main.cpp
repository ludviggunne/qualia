
#include <assert.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm_config.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "shaders.h"
#include "vao.h"
#include "log.h"
#include "texture.h"

// Size of work groups
#define LOCAL_INVOCATION_SIZE 32

// Initial window size
#define WINDOW_WIDTH     800
#define WINDOW_HEIGHT    600

#define CANVAS_MARGIN    50
#define MAX_CANVAS_WIDTH 4096 

#define BIND_KEY(key, action) if (glfwGetKey(window, key) == GLFW_PRESS) { action; }

uint32_t window_w, window_h;

Vertex vertex_data[4];
VertexArray<Vertex> *vao = nullptr;
glm::mat4 uView;
Texture *texture      = nullptr;
uint32_t texture_size = 128;
uint32_t upscale_size = 512;
uint32_t save_size    = 1024;
uint32_t patch_size   = 512;
Program *compute_prgm;
bool reset_tex_flag   = false;

struct {

    glm::vec4 seed       { 0.0f, 0.0f, 0.0f, 0.0f };
    float     section    = 0.0f;
    int32_t   iterations = 40;
    float     rotation   = 0.0f;
    float     zoom       = 2.5f;
    glm::vec2 center     { 0.0f, 0.0f };
    glm::vec2 clipping   {-1.0f, 1.0f };

} settings;

struct {

    int32_t
        view,
        sampler,

        seed,
        section,
        iterations,
        zoom,
        center,
        rotation,
        clipping,
        
        offset,
        size;

} uniforms;

uint32_t canvas_width(uint32_t w, uint32_t h, uint32_t margin, uint32_t max) {

    uint32_t mw = w - 2 * margin;
    uint32_t mh = h - 2 * margin;
    uint32_t m = mw > mh ? mh : mw;
    m = m > max ? max : m;

    return m;
}

void set_canvas_vertex_data(Vertex *v, uint32_t w, uint32_t h) {

    auto cw = canvas_width(w, h, CANVAS_MARGIN, MAX_CANVAS_WIDTH);

    *v = {
        glm::vec2 { (w - cw) / 2, (h - cw) / 2 },
        glm::vec2 { 0.0, 1.0 }
    };
    *(v + 1) = {
        glm::vec2 { (w + cw) / 2, (h - cw) / 2 },
        glm::vec2 { 1.0, 1.0 }
    };
    *(v + 2) = {
        glm::vec2 { (w - cw) / 2, (h + cw) / 2 },
        glm::vec2 { 0.0, 0.0 }
    };
    *(v + 3) = {
        glm::vec2 { (w + cw) / 2, (h + cw) / 2 },
        glm::vec2 { 1.0, 0.0 }
    };
}

void resize_callback(GLFWwindow *window, int w, int h) {

    GL_CALL( glViewport(0, 0, w, h) );

    set_canvas_vertex_data(vertex_data, w, h);
    vao->vertex_data(vertex_data, 4);
    uView = glm::ortho(0.0f, (float)w, 0.0f, (float)h, -1.0f, 100.0f);
}

void upscale_texture(uint32_t size);
int create_window(GLFWwindow **window) {

    if (!glfwInit()) return 0;

    *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "QUALIA", NULL, NULL);

    if (*window == NULL) return 0;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSwapInterval(0);
    glfwMakeContextCurrent(*window);

    glfwSetWindowSizeCallback(*window, &resize_callback);
    glfwSetKeyCallback(*window, [](GLFWwindow *win, int key, int _0, int action, int _1) {
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) upscale_texture(upscale_size);
    });

    return 1;
}

void setup_texture(uint32_t w, uint32_t h) {

    if (texture) delete texture;
    texture = new Texture(w, h, GL_RGBA16F);
    texture->bind_to_unit(0);
    texture->set_parameter(GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    texture->set_parameter(GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    texture->set_parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    texture->set_parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    texture->load_image(nullptr, GL_RGBA, GL_FLOAT);
    texture->bind_as_image(0);

    compute_prgm->use();
    compute_prgm->setUniform(glm::ivec2 { w, h }, uniforms.size);
    reset_tex_flag = false;
}

void dispatch_compute();
void upscale_texture(uint32_t size) {

    auto tmp_size = texture_size;
    
    texture_size = size;
    setup_texture(texture_size, texture_size);
    dispatch_compute();
   
    texture_size = tmp_size;
    reset_tex_flag = true;
}

void save_texture() {

    auto img = new uint8_t[upscale_size * upscale_size * 4];
    upscale_texture(save_size);
    texture->get_image(img, GL_RGBA);
    stbi_write_png("output.png", upscale_size, upscale_size, 4, img, 0);
    delete img;
}

void dispatch_compute() {

    compute_prgm->use();
    compute_prgm->setUniform(settings.seed,       uniforms.seed);
    compute_prgm->setUniform(settings.section,    uniforms.section);
    compute_prgm->setUniform(settings.iterations, uniforms.iterations);
    compute_prgm->setUniform(settings.rotation,   uniforms.rotation);
    compute_prgm->setUniform(settings.zoom,       uniforms.zoom);
    compute_prgm->setUniform(settings.center,     uniforms.center);
    compute_prgm->setUniform(settings.clipping,   uniforms.clipping);
    compute_prgm->setUniform(glm::ivec2 { 0, 0 }, uniforms.offset);

    if (texture_size > patch_size) {

        for (int32_t u = 0; u < static_cast<int32_t>(texture_size); u += patch_size) {

            for (int32_t v = 0; v < static_cast<int32_t>(texture_size); v += patch_size) {

                compute_prgm->setUniform(glm::ivec2 { u, v }, uniforms.offset);
                GL_CALL( glDispatchCompute(patch_size / LOCAL_INVOCATION_SIZE, patch_size / LOCAL_INVOCATION_SIZE, 1) );
                GL_CALL( glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT) );
                GL_CALL( glFinish() );
            }
        }
    } else {

        GL_CALL( glDispatchCompute(texture_size / LOCAL_INVOCATION_SIZE, texture_size / LOCAL_INVOCATION_SIZE, 1) );
        GL_CALL( glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT) );
        GL_CALL( glFinish() );
    }
}

int main(int argc, char *argv[]) {

    GLFWwindow *window; 
    create_window(&window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    GL_QUERY_INT_IV(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0);
    GL_QUERY_INT_IV(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1);
    GL_QUERY_INT_IV(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2);
    GL_QUERY_INT_IV(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0);
    GL_QUERY_INT_IV(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1);
    GL_QUERY_INT_IV(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2);
    GL_QUERY_INT_IV(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 0);
    GL_QUERY_INT_IV(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 1);
    GL_QUERY_INT_IV(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 2);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    auto &style = ImGui::GetStyle();
    style.FrameRounding = 5.0f;
    style.WindowRounding = 5.0f;
    

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450 core");

    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);



    auto vsrc = loadFile("simple_shader.vert.glsl");
    auto fsrc = loadFile("simple_shader.frag.glsl");
    auto csrc = loadFile("generate.comp.glsl");

    assert(!vsrc.empty());
    assert(!fsrc.empty());
    assert(!csrc.empty());

    Shader vs(GL_VERTEX_SHADER);
    Shader fs(GL_FRAGMENT_SHADER);
    Shader cs(GL_COMPUTE_SHADER);

    auto vlog = vs.compile(vsrc);
    auto flog = fs.compile(fsrc);
    auto clog = cs.compile(csrc);

    if (!vlog.empty()) debugPrint("Vertex Shader Compile error: "   << vlog);
    if (!flog.empty()) debugPrint("Fragment Shader Compile error: " << flog);
    if (!clog.empty()) debugPrint("Compute Shader Compile error: "  << clog);

    Program render_prgm;
    compute_prgm = new Program;

    render_prgm.attach(vs);
    render_prgm.attach(fs);
    compute_prgm->attach(cs);

    auto rplog = render_prgm.link();
    auto cplog = compute_prgm->link();

    if (!rplog.empty()) debugPrint("Render Program Link error: "  << rplog);
    if (!cplog.empty()) debugPrint("Compute Program Link error: " << cplog);

    uniforms.view       = render_prgm.queryUniformLocation("uView");
    uniforms.sampler    = render_prgm.queryUniformLocation("uSampler");
    uniforms.seed       = compute_prgm->queryUniformLocation("uSeed");
    uniforms.section    = compute_prgm->queryUniformLocation("uSection");
    uniforms.iterations = compute_prgm->queryUniformLocation("uIterations");
    uniforms.zoom       = compute_prgm->queryUniformLocation("uZoom");
    uniforms.center     = compute_prgm->queryUniformLocation("uCenter");
    uniforms.rotation   = compute_prgm->queryUniformLocation("uRotation");
    uniforms.clipping   = compute_prgm->queryUniformLocation("uClipping");
    uniforms.offset     = compute_prgm->queryUniformLocation("uOffset");
    uniforms.size       = compute_prgm->queryUniformLocation("uSize");

    assert(uniforms.view       != -1);
    assert(uniforms.sampler    != -1);
    assert(uniforms.seed       != -1);
    assert(uniforms.section    != -1);
    assert(uniforms.iterations != -1);
    assert(uniforms.zoom       != -1);
    assert(uniforms.center     != -1);
    assert(uniforms.rotation   != -1);
    assert(uniforms.clipping   != -1);
    assert(uniforms.offset     != -1);
    assert(uniforms.size       != -1);

    render_prgm.use();
    render_prgm.setUniform<int32_t>(0, uniforms.sampler);


    setup_texture(texture_size, texture_size);


    vao = new VertexArray<Vertex>;

    uint32_t idata[] = {

        0, 1, 2,
        1, 2, 3
    };

    vao->index_data(idata, 6);
    resize_callback(window, WINDOW_WIDTH, WINDOW_HEIGHT);


    bool first_frame = true;
    while (!glfwWindowShouldClose(window)) {
        
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        render_prgm.use();
        render_prgm.setUniform(uView, uniforms.view);
        vao->draw(GL_TRIANGLES);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        bool has_changed = false;
        {
            ImGui::Begin("Settings");

            static const auto speed = 0.001f;
            has_changed |= ImGui::DragFloat4("Seed",     &settings.seed.x,     speed, -1.0f,   1.0f);
            has_changed |= ImGui::DragFloat("Section",   &settings.section,    speed, -1.0f,   1.0f);
            has_changed |= ImGui::DragInt("Iterations",  &settings.iterations, 0.2f,   4,      100);
            has_changed |= ImGui::DragFloat("Rotation",  &settings.rotation,   speed,  0.0f,   6.29f);
            has_changed |= ImGui::DragFloat("Zoom",      &settings.zoom,       speed,  0.125f, 8.0f);
            has_changed |= ImGui::DragFloat2("Center",   &settings.center.x,   speed, -4.0f,   4.0f);
            has_changed |= ImGui::DragFloat2("Clipping", &settings.clipping.x, speed, -2.0f,   2.0f);

            if (ImGui::Button("Upscale")) {

                upscale_texture(upscale_size);
            }
            ImGui::SameLine();
            if (ImGui::Button("Save")) {

                save_texture();
            }

            ImGui::End();
        }

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        {
            static const float rot_speed  = 0.01f;
            static const float pan_speed  = 0.01f;
            static const float zoom_speed = 1.02f;
            static const float seed_speed = 0.005f;

            BIND_KEY(GLFW_KEY_UP,     has_changed = true; settings.center.y -= settings.zoom * pan_speed);
            BIND_KEY(GLFW_KEY_DOWN,   has_changed = true; settings.center.y += settings.zoom * pan_speed);
            BIND_KEY(GLFW_KEY_LEFT,   has_changed = true; settings.center.x -= settings.zoom * pan_speed);
            BIND_KEY(GLFW_KEY_RIGHT,  has_changed = true; settings.center.x += settings.zoom * pan_speed);
            BIND_KEY(GLFW_KEY_A,      has_changed = true; settings.rotation -= rot_speed);
            BIND_KEY(GLFW_KEY_D,      has_changed = true; settings.rotation += rot_speed);
            BIND_KEY(GLFW_KEY_W,      has_changed = true; settings.zoom     /= zoom_speed);
            BIND_KEY(GLFW_KEY_S,      has_changed = true; settings.zoom     *= zoom_speed);
            BIND_KEY(GLFW_KEY_Y,      has_changed = true; settings.seed.x   += seed_speed);
            BIND_KEY(GLFW_KEY_H,      has_changed = true; settings.seed.x   -= seed_speed);
            BIND_KEY(GLFW_KEY_U,      has_changed = true; settings.seed.y   += seed_speed);
            BIND_KEY(GLFW_KEY_J,      has_changed = true; settings.seed.y   -= seed_speed);
            BIND_KEY(GLFW_KEY_I,      has_changed = true; settings.seed.z   += seed_speed);
            BIND_KEY(GLFW_KEY_K,      has_changed = true; settings.seed.z   -= seed_speed);
            BIND_KEY(GLFW_KEY_O,      has_changed = true; settings.seed.w   += seed_speed);
            BIND_KEY(GLFW_KEY_L,      has_changed = true; settings.seed.w   -= seed_speed);
            BIND_KEY(GLFW_KEY_ESCAPE, glfwSetWindowShouldClose(window, GLFW_TRUE));

        }


        if (has_changed || first_frame) {

            if (reset_tex_flag) {

                setup_texture(texture_size, texture_size);
            }


            dispatch_compute();    
        }

        glfwPollEvents();
        glfwSwapBuffers(window);

        first_frame = false;
    }

    delete compute_prgm;
    delete texture;
    delete vao;

    glfwTerminate();

    return 0;
}