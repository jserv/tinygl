#!/usr/bin/env bash

# TinyGL Consolidated Test Suite
# Usage: ./tests/driver.sh [OPTIONS]
#
# Options:
#   --generate-expected  Generate reference checksums
#   --api               Run API coverage tests only
#   --regression        Run regression tests only
#   --perf              Run performance tests only
#   --verbose           Show compilation errors
#   -h, --help          Show this help

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
EXPECTED_DIR="$SCRIPT_DIR/expected"
TMP_DIR="/tmp/tinygl_tests_$$"

# Colors (disabled if not terminal)
if [ -t 1 ]; then
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    YELLOW='\033[0;33m'
    BLUE='\033[0;34m'
    NC='\033[0m'
else
    GREEN='' RED='' YELLOW='' BLUE='' NC=''
fi

PASS=0
FAIL=0
SKIP=0
GENERATE_EXPECTED=0
VERBOSE=0
RUN_API=1
RUN_REGRESSION=1
RUN_PERF=1

# Parse arguments
for arg in "$@"; do
    case "$arg" in
        --generate-expected) GENERATE_EXPECTED=1 ;;
        --api) RUN_API=1; RUN_REGRESSION=0; RUN_PERF=0 ;;
        --regression) RUN_API=0; RUN_REGRESSION=1; RUN_PERF=0 ;;
        --perf) RUN_API=0; RUN_REGRESSION=0; RUN_PERF=1 ;;
        --verbose) VERBOSE=1 ;;
        --help|-h)
            sed -n '3,12p' "$0" | sed 's/^# //'
            exit 0
            ;;
    esac
done

log_pass() { printf "[ %bOK%b ]\n" "$GREEN" "$NC"; PASS=$((PASS + 1)); }
log_fail() { printf "%bFAIL%b: %s - %s\n" "$RED" "$NC" "$1" "$2"; FAIL=$((FAIL + 1)); }
log_skip() { printf "%bSKIP%b: %s - %s\n" "$YELLOW" "$NC" "$1" "$2"; SKIP=$((SKIP + 1)); }
log_perf() { printf "%bPERF%b: %s - %s\n" "$BLUE" "$NC" "$1" "$2"; PASS=$((PASS + 1)); }

# Compiler settings
CC="${CC:-cc}"
CFLAGS="-std=c99 -O2 -I$PROJECT_DIR/include"
LDFLAGS="$PROJECT_DIR/lib/libTinyGL.a -lm"

# Create temp directory
mkdir -p "$TMP_DIR"
trap "rm -rf $TMP_DIR" EXIT

# Compile and run a test program
# Usage: run_test "name" "source_code" [expected_exit_code]
run_test() {
    local name="$1"
    local source="$2"
    local expected_exit="${3:-0}"
    local src_file="$TMP_DIR/${name}.c"
    local bin_file="$TMP_DIR/${name}"

    printf "  %-40s " "$name..."

    echo "$source" > "$src_file"

    local compile_out
    if compile_out=$($CC $CFLAGS "$src_file" $LDFLAGS -o "$bin_file" 2>&1); then
        if "$bin_file" >/dev/null 2>&1; then
            local actual_exit=$?
            if [ "$actual_exit" -eq "$expected_exit" ]; then
                log_pass "$name"
                return 0
            else
                log_fail "$name" "exit code $actual_exit (expected $expected_exit)"
                return 1
            fi
        else
            local actual_exit=$?
            if [ "$expected_exit" -ne 0 ] && [ "$actual_exit" -ne 0 ]; then
                log_pass "$name (expected failure)"
                return 0
            fi
            log_fail "$name" "runtime error (exit $actual_exit)"
            return 1
        fi
    else
        if [ "$VERBOSE" -eq 1 ]; then
            echo "$compile_out" >&2
        fi
        log_fail "$name" "compilation failed"
        return 1
    fi
}

# Run a performance benchmark
# Usage: run_perf_test "name" "source_code" "metric_description"
run_perf_test() {
    local name="$1"
    local source="$2"
    local metric="$3"
    local src_file="$TMP_DIR/${name}.c"
    local bin_file="$TMP_DIR/${name}"

    printf "  %-40s " "$name..."

    echo "$source" > "$src_file"

    if $CC $CFLAGS "$src_file" $LDFLAGS -o "$bin_file" 2>/dev/null; then
        local result
        result=$("$bin_file" 2>&1)
        if [ $? -eq 0 ]; then
            log_perf "$name" "$result $metric"
            return 0
        else
            log_fail "$name" "runtime error"
            return 1
        fi
    else
        log_fail "$name" "compilation failed"
        return 1
    fi
}

# SHA256 computation (portable)
sha256_file() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1" | cut -d' ' -f1
    elif command -v shasum >/dev/null 2>&1; then
        shasum -a 256 "$1" | cut -d' ' -f1
    else
        echo "ERROR: no sha256 tool found" >&2
        exit 1
    fi
}

cd "$PROJECT_DIR"

echo "=== TinyGL Consolidated Test Suite ==="
echo ""

# Build tests
echo "--- Build Tests ---"

printf "  %-40s " "library build..."
if make -s clean >/dev/null 2>&1 && make -s all >/dev/null 2>&1; then
    if [ -f lib/libTinyGL.a ]; then
        log_pass "library build"
    else
        log_fail "library build" "libTinyGL.a not found"
    fi
else
    log_fail "library build" "compilation failed"
    echo "FATAL: Cannot continue without library" >&2
    exit 1
fi

printf "  %-40s " "raw examples build..."
if make -s raw_examples >/dev/null 2>&1; then
    if [ -x examples/raw/gears ]; then
        log_pass "raw examples build"
    else
        log_fail "raw examples build" "gears executable not found"
    fi
else
    log_fail "raw examples build" "compilation failed"
fi

echo ""

# Rendering tests
echo "--- Rendering Tests ---"

printf "  %-40s " "gears render..."
cd "$PROJECT_DIR/examples/raw"
rm -f render.png

if ./gears >/dev/null 2>&1; then
    if [ -f render.png ]; then
        ACTUAL_SHA=$(sha256_file render.png)
        EXPECTED_FILE="$EXPECTED_DIR/gears_render.sha256"

        if [ $GENERATE_EXPECTED -eq 1 ]; then
            mkdir -p "$EXPECTED_DIR"
            echo "$ACTUAL_SHA" > "$EXPECTED_FILE"
            log_pass "gears render (reference generated)"
        elif [ -f "$EXPECTED_FILE" ]; then
            EXPECTED_SHA=$(cat "$EXPECTED_FILE")
            if [ "$ACTUAL_SHA" = "$EXPECTED_SHA" ]; then
                log_pass "gears render"
            else
                log_fail "gears render" "output mismatch"
            fi
        else
            log_skip "gears render" "no reference (run with --generate-expected)"
        fi
    else
        log_fail "gears render" "render.png not created"
    fi
else
    log_fail "gears render" "execution failed"
fi

cd "$PROJECT_DIR"
echo ""

# API coverage tests
if [ "$RUN_API" -eq 1 ]; then
echo "--- API Coverage Tests ---"

# Common test header
API_HEADER='
#include <stddef.h>
#include <stdlib.h>
#include <TGL/gl.h>
#include "zbuffer.h"

static ZBuffer *zb;
static void setup(void) {
    zb = ZB_open(128, 128, ZB_MODE_RGBA, NULL);
    if (!zb) exit(1);
    glInit(zb);
}
static void teardown(void) {
    ZB_close(zb);
    glClose();
}
'

# Test: Basic primitives (GL_TRIANGLES, GL_QUADS, GL_LINES, GL_POINTS)
run_test "api_primitives" "$API_HEADER
int main(void) {
    setup();

    /* Triangles */
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0); glVertex3f(1, 0, 0); glVertex3f(0, 1, 0);
    glEnd();

    /* Quads */
    glBegin(GL_QUADS);
    glVertex3f(0, 0, 0); glVertex3f(1, 0, 0);
    glVertex3f(1, 1, 0); glVertex3f(0, 1, 0);
    glEnd();

    /* Lines */
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0); glVertex3f(1, 1, 0);
    glEnd();

    /* Points */
    glBegin(GL_POINTS);
    glVertex3f(0.5f, 0.5f, 0);
    glEnd();

    teardown();
    return 0;
}"

# Test: Triangle strips and fans
run_test "api_strips_fans" "$API_HEADER
int main(void) {
    setup();

    /* Triangle strip */
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(0, 0, 0); glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0); glVertex3f(1, 1, 0);
    glEnd();

    /* Triangle fan */
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.5f, 0.5f, 0);  /* center */
    glVertex3f(0, 0, 0); glVertex3f(1, 0, 0);
    glVertex3f(1, 1, 0); glVertex3f(0, 1, 0);
    glEnd();

    teardown();
    return 0;
}"

# Test: Matrix operations
run_test "api_matrix" "$API_HEADER
int main(void) {
    setup();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    glTranslatef(1.0f, 2.0f, 3.0f);
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
    glScalef(2.0f, 2.0f, 2.0f);
    glPopMatrix();

    teardown();
    return 0;
}"

# Test: Lighting
run_test "api_lighting" "$API_HEADER
int main(void) {
    setup();

    GLfloat pos[] = {1.0f, 1.0f, 1.0f, 0.0f};
    GLfloat amb[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat diff[] = {0.8f, 0.8f, 0.8f, 1.0f};

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);

    teardown();
    return 0;
}"

# Test: Materials
run_test "api_materials" "$API_HEADER
int main(void) {
    setup();

    GLfloat amb[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat diff[] = {0.8f, 0.0f, 0.0f, 1.0f};
    GLfloat spec[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);

    teardown();
    return 0;
}"

# Test: Texturing (TinyGL requires 256x256 textures by default)
run_test "api_texturing" "$API_HEADER
#define TEX_SIZE 256
int main(void) {
    static unsigned char pixels[TEX_SIZE * TEX_SIZE * 3];
    GLuint tex;
    int i;

    setup();

    for (i = 0; i < (int)sizeof(pixels); i++) pixels[i] = (i * 17) & 0xFF;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(1, 0, 0);
    glTexCoord2f(0, 1); glVertex3f(0, 1, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glDeleteTextures(1, &tex);
    teardown();
    return 0;
}"

# Test: Viewport and clear
run_test "api_viewport_clear" "$API_HEADER
int main(void) {
    setup();

    glViewport(0, 0, 128, 128);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    teardown();
    return 0;
}"

# Test: Depth testing
run_test "api_depth" "$API_HEADER
int main(void) {
    setup();

    glEnable(GL_DEPTH_TEST);

    /* Draw two overlapping triangles at different depths */
    glBegin(GL_TRIANGLES);
    glColor3f(1, 0, 0);
    glVertex3f(0.2f, 0.2f, 0.5f);
    glVertex3f(0.8f, 0.2f, 0.5f);
    glVertex3f(0.5f, 0.8f, 0.5f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0, 0, 1);
    glVertex3f(0.3f, 0.3f, 0.3f);
    glVertex3f(0.7f, 0.3f, 0.3f);
    glVertex3f(0.5f, 0.7f, 0.3f);
    glEnd();

    glDisable(GL_DEPTH_TEST);
    teardown();
    return 0;
}"

# Test: Culling
run_test "api_culling" "$API_HEADER
int main(void) {
    setup();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0);
    glEnd();

    glDisable(GL_CULL_FACE);
    teardown();
    return 0;
}"

# Test: Blending
run_test "api_blending" "$API_HEADER
int main(void) {
    setup();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0);
    glEnd();

    glDisable(GL_BLEND);
    teardown();
    return 0;
}"

# Test: Display lists
run_test "api_display_lists" "$API_HEADER
int main(void) {
    setup();

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0);
    glEnd();
    glEndList();

    glCallList(list);
    glDeleteLists(list, 1);

    teardown();
    return 0;
}"

# Test: Polygon modes
run_test "api_polygon_modes" "$API_HEADER
int main(void) {
    setup();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0); glVertex3f(1, 0, 0); glVertex3f(0, 1, 0);
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0); glVertex3f(1, 0, 0); glVertex3f(0, 1, 0);
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0); glVertex3f(1, 0, 0); glVertex3f(0, 1, 0);
    glEnd();

    teardown();
    return 0;
}"

# Test: Color variants
run_test "api_color_variants" "$API_HEADER
int main(void) {
    setup();

    glBegin(GL_TRIANGLES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0, 0, 0);

    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    glVertex3f(1, 0, 0);

    GLfloat c3[] = {0.0f, 0.0f, 1.0f};
    glColor3fv(c3);
    glVertex3f(0, 1, 0);

    glEnd();
    teardown();
    return 0;
}"

# Test: Normal variants
run_test "api_normal_variants" "$API_HEADER
int main(void) {
    setup();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0, 0, 0);

    GLfloat n[] = {0.0f, 0.0f, 1.0f};
    glNormal3fv(n);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0);
    glEnd();

    glDisable(GL_LIGHTING);
    teardown();
    return 0;
}"

# Test: Frustum projection
run_test "api_projections" "$API_HEADER
int main(void) {
    setup();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Draw with perspective */
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, -5);
    glVertex3f(1, 0, -5);
    glVertex3f(0, 1, -5);
    glEnd();

    teardown();
    return 0;
}"

# Test: Point size
run_test "api_point_size" "$API_HEADER
int main(void) {
    int i;
    setup();

    /* Test various point sizes */
    for (i = 1; i <= 5; i++) {
        glPointSize((float)i * 2.0f);
        glBegin(GL_POINTS);
        glColor3f((float)i / 5.0f, 0.5f, 0.5f);
        glVertex3f(0.1f * i, 0.5f, 0);
        glEnd();
    }

    teardown();
    return 0;
}"

echo ""
fi

# Regression tests
if [ "$RUN_REGRESSION" -eq 1 ]; then
echo "--- Regression Tests ---"

# Common header for regression tests
REG_HEADER='
#include <stddef.h>
#include <stdlib.h>
#include <TGL/gl.h>
#include "zbuffer.h"

static ZBuffer *zb;
static void setup(void) {
    zb = ZB_open(128, 128, ZB_MODE_RGBA, NULL);
    if (!zb) exit(1);
    glInit(zb);
}
static void teardown(void) {
    ZB_close(zb);
    glClose();
}
'

# Test: Init/close cycles (memory leak detection)
run_test "reg_init_close_cycles" '
#include <stddef.h>
#include <TGL/gl.h>
#include "zbuffer.h"

int main(void) {
    int i;
    for (i = 0; i < 20; i++) {
        ZBuffer *zb = ZB_open(64, 64, ZB_MODE_RGBA, NULL);
        if (!zb) return 1;
        glInit(zb);

        glBegin(GL_TRIANGLES);
        glVertex3f(0, 0, 0);
        glVertex3f(1, 0, 0);
        glVertex3f(0, 1, 0);
        glEnd();

        ZB_close(zb);
        glClose();
    }
    return 0;
}'

# Test: Zero-length line (IMPROVE.md: division by zero)
run_test "reg_zero_length_line" "$REG_HEADER
int main(void) {
    setup();

    glBegin(GL_LINES);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glVertex3f(0.5f, 0.5f, 0.0f);  /* zero length */
    glEnd();

    teardown();
    return 0;
}"

# Test: Degenerate triangle (zero area)
run_test "reg_degenerate_triangle" "$REG_HEADER
int main(void) {
    setup();

    /* Collinear points - zero area triangle */
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.5f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glEnd();

    /* Single point repeated */
    glBegin(GL_TRIANGLES);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glEnd();

    teardown();
    return 0;
}"

# Test: Large coordinate values (viewport overflow protection)
run_test "reg_large_coordinates" "$REG_HEADER
int main(void) {
    setup();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 0.1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Very large coordinates that might overflow */
    glBegin(GL_TRIANGLES);
    glVertex3f(1e6f, 1e6f, -10.0f);
    glVertex3f(-1e6f, 1e6f, -10.0f);
    glVertex3f(0.0f, -1e6f, -10.0f);
    glEnd();

    teardown();
    return 0;
}"

# Test: Deep matrix stack
run_test "reg_matrix_stack_depth" "$REG_HEADER
int main(void) {
    int i;
    setup();

    glMatrixMode(GL_MODELVIEW);

    /* Push many matrices (test stack limits) */
    for (i = 0; i < 16; i++) {
        glPushMatrix();
        glTranslatef(0.1f, 0.1f, 0.1f);
    }

    /* Pop them all */
    for (i = 0; i < 16; i++) {
        glPopMatrix();
    }

    teardown();
    return 0;
}"

# Test: Many vertices in single primitive
run_test "reg_many_vertices" "$REG_HEADER
#include <math.h>
int main(void) {
    int i;
    setup();

    /* Many triangles in sequence */
    for (i = 0; i < 50; i++) {
        float x = (float)(i % 10) * 0.1f;
        float y = (float)(i / 10) * 0.2f;
        glBegin(GL_TRIANGLES);
        glVertex3f(x, y, 0);
        glVertex3f(x + 0.08f, y, 0);
        glVertex3f(x + 0.04f, y + 0.15f, 0);
        glEnd();
    }

    teardown();
    return 0;
}"

# Test: Rapid texture bind/unbind (256x256 RGB required)
run_test "reg_texture_thrashing" "$REG_HEADER
#define TEX_SIZE 256
int main(void) {
    static unsigned char pixels[TEX_SIZE * TEX_SIZE * 3];
    int i;
    GLuint tex[4];

    setup();

    for (i = 0; i < (int)sizeof(pixels); i++) pixels[i] = i & 0xFF;

    glGenTextures(4, tex);
    for (i = 0; i < 4; i++) {
        glBindTexture(GL_TEXTURE_2D, tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    }

    /* Rapid switching */
    for (i = 0; i < 100; i++) {
        glBindTexture(GL_TEXTURE_2D, tex[i % 4]);
    }

    glDeleteTextures(4, tex);
    teardown();
    return 0;
}"

# Test: Enable/disable state toggling
run_test "reg_state_toggling" "$REG_HEADER
int main(void) {
    int i;
    setup();

    for (i = 0; i < 50; i++) {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);

        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
    }

    teardown();
    return 0;
}"

# Test: Empty primitive
run_test "reg_empty_primitive" "$REG_HEADER
int main(void) {
    setup();

    /* Begin/end with no vertices */
    glBegin(GL_TRIANGLES);
    glEnd();

    glBegin(GL_LINES);
    glEnd();

    glBegin(GL_POINTS);
    glEnd();

    teardown();
    return 0;
}"

# Test: Incomplete primitive
run_test "reg_incomplete_primitive" "$REG_HEADER
int main(void) {
    setup();

    /* Triangle with only 2 vertices */
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glEnd();

    /* Quad with only 3 vertices */
    glBegin(GL_QUADS);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(1, 1, 0);
    glEnd();

    teardown();
    return 0;
}"

# Test: Polygon stipple (if enabled)
run_test "reg_polygon_stipple" "$REG_HEADER
int main(void) {
    GLubyte pattern[128];
    int i;

    setup();

    for (i = 0; i < 128; i++) pattern[i] = (i & 1) ? 0xAA : 0x55;

    glPolygonStipple(pattern);
    glEnable(GL_POLYGON_STIPPLE);

    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0);
    glEnd();

    glDisable(GL_POLYGON_STIPPLE);
    teardown();
    return 0;
}"

echo ""
fi

# Performance tests
if [ "$RUN_PERF" -eq 1 ]; then
echo "--- Performance Tests ---"

PERF_HEADER='
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <TGL/gl.h>
#include "zbuffer.h"

static ZBuffer *zb;
static void setup(int w, int h) {
    zb = ZB_open(w, h, ZB_MODE_RGBA, NULL);
    if (!zb) exit(1);
    glInit(zb);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* Use frustum since glOrtho is a stub */
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2.0f);  /* Move into view */
}
static void teardown(void) {
    ZB_close(zb);
    glClose();
}
static double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
'

# Perf: Triangle throughput
run_perf_test "perf_triangle_throughput" "$PERF_HEADER
int main(void) {
    int i;
    double start, elapsed;
    int count = 100000;

    setup(256, 256);
    glClear(GL_COLOR_BUFFER_BIT);

    start = get_time();
    for (i = 0; i < count; i++) {
        glBegin(GL_TRIANGLES);
        glColor3f((float)(i%256)/255.0f, 0.5f, 0.5f);
        glVertex3f(0.2f, 0.2f, 0);
        glVertex3f(0.8f, 0.2f, 0);
        glVertex3f(0.5f, 0.8f, 0);
        glEnd();
    }
    elapsed = get_time() - start;

    printf(\"%.0f\", count / elapsed);
    teardown();
    return 0;
}" "triangles/sec"

# Perf: Line throughput
run_perf_test "perf_line_throughput" "$PERF_HEADER
int main(void) {
    int i;
    double start, elapsed;
    int count = 200000;

    setup(256, 256);
    glClear(GL_COLOR_BUFFER_BIT);

    start = get_time();
    glBegin(GL_LINES);
    for (i = 0; i < count; i++) {
        float t = (float)i / count;
        glVertex3f(t, 0.0f, 0);
        glVertex3f(1.0f - t, 1.0f, 0);
    }
    glEnd();
    elapsed = get_time() - start;

    printf(\"%.0f\", count / elapsed);
    teardown();
    return 0;
}" "lines/sec"

# Perf: Clear buffer speed
run_perf_test "perf_clear_speed" "$PERF_HEADER
int main(void) {
    int i;
    double start, elapsed;
    int count = 1000;

    setup(512, 512);

    start = get_time();
    for (i = 0; i < count; i++) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    elapsed = get_time() - start;

    printf(\"%.1f\", count / elapsed);
    teardown();
    return 0;
}" "clears/sec (512x512)"

# Perf: Matrix operations
run_perf_test "perf_matrix_ops" "$PERF_HEADER
int main(void) {
    int i;
    double start, elapsed;
    int count = 100000;

    setup(64, 64);

    start = get_time();
    for (i = 0; i < count; i++) {
        glPushMatrix();
        glTranslatef(0.1f, 0.1f, 0.1f);
        glRotatef(1.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.01f, 1.01f, 1.01f);
        glPopMatrix();
    }
    elapsed = get_time() - start;

    printf(\"%.0f\", count / elapsed);
    teardown();
    return 0;
}" "matrix-ops/sec"

# Perf: Textured triangles (256x256 RGB required)
run_perf_test "perf_textured_triangles" "$PERF_HEADER
#define TEX_SIZE 256
int main(void) {
    static unsigned char pixels[TEX_SIZE * TEX_SIZE * 3];
    int i;
    double start, elapsed;
    int count = 50000;
    GLuint tex;

    setup(256, 256);

    for (i = 0; i < (int)sizeof(pixels); i++) pixels[i] = i & 0xFF;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glEnable(GL_TEXTURE_2D);

    glClear(GL_COLOR_BUFFER_BIT);

    start = get_time();
    for (i = 0; i < count; i++) {
        glBegin(GL_TRIANGLES);
        glTexCoord2f(0, 0); glVertex3f(0.2f, 0.2f, 0);
        glTexCoord2f(1, 0); glVertex3f(0.8f, 0.2f, 0);
        glTexCoord2f(0.5f, 1); glVertex3f(0.5f, 0.8f, 0);
        glEnd();
    }
    elapsed = get_time() - start;

    printf(\"%.0f\", count / elapsed);
    glDeleteTextures(1, &tex);
    teardown();
    return 0;
}" "tex-tris/sec"

echo ""
fi

# ============================================================
# SUMMARY
# ============================================================
echo "=== Summary ==="
TOTAL=$((PASS + FAIL + SKIP))
echo "  Total: $TOTAL tests"
printf "  %bPassed%b: %d\n" "$GREEN" "$NC" "$PASS"
[ $FAIL -gt 0 ] && printf "  %bFailed%b: %d\n" "$RED" "$NC" "$FAIL"
[ $SKIP -gt 0 ] && printf "  %bSkipped%b: %d\n" "$YELLOW" "$NC" "$SKIP"
echo ""

# Exit with failure if any tests failed
[ $FAIL -eq 0 ]
