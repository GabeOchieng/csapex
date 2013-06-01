/*
 * evaluator.cpp
 *
 *  Created on: Feb 01, 2013
 *      Author: buck <sebastian.buck@uni-tuebingen.de>
 */

/// HEADER
#include "evaluator.h"

/// PROJECT
#include <utils/LibUtil/Stopwatch.h>

/// SYSTEM
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <yaml-cpp/yaml.h>

using namespace lib_path;

namespace
{
void mouse_cb(int event, int x, int y, int flags, void* userdata)
{
    MouseCallback* eval = static_cast<MouseCallback*>(userdata);
    eval->mouseCallback(event, x, y, flags);
}
}

template <class Search>
void Evaluator<Search>::mouseCallback(int event, int x, int y, int flags)
{
    bool refresh = true;
    bool is_dragging = false;
    bool up = event == CV_EVENT_LBUTTONUP || event == CV_EVENT_RBUTTONUP;

    if(event == CV_EVENT_LBUTTONDOWN) {
        drag = DRAG_LEFT;
        drag_start.x = x;
        drag_start.y = y;

    } else if(event == CV_EVENT_RBUTTONDOWN) {
        drag = DRAG_RIGHT;
        drag_start.x = x;
        drag_start.y = y;

    } else if(up) {
        switch(drag_mode) {
        case SET_FOCUS:
            setFocus(x, y);
            algo.setFocus(circle_center.x, circle_center.y, circle_radius);
            complete_repaint_ = true;
            break;
        case SET_OBSTACLE:
        {
            selection.width = -1;
            selection.height = -1;
            cv::Scalar col = drag == DRAG_LEFT ? cv::Scalar::all(0) : cv::Scalar::all(255);
            cv::rectangle(orig_map, cv::Point(drag_start.x / SCALE, h- drag_start.y / SCALE), cv::Point(x / SCALE, h- y / SCALE), col,
                          CV_FILLED, 8, 0);
            generateMap();
            complete_repaint_ = true;
            refresh = true;
            break;
        }
        case MAKE_MAZE:
        {
            makeMaze(selection.width);
            selection.width = -1;
            selection.height = -1;
            complete_repaint_ = true;
            refresh = true;
            break;
        }
        }

        is_dragging = true;
        drag = DRAG_NONE;

    } else if(event == CV_EVENT_MOUSEMOVE && drag != DRAG_NONE) {
        is_dragging = true;
    } else {
        refresh = false;
    }

    if(is_dragging) {
        switch(drag_mode) {
        case SET_FOCUS:
            setFocus(x, y);
            break;
        case SET_START:
            start.x = drag_start.x / SCALE;
            start.y = h- drag_start.y / SCALE;
            start.theta = std::atan2(drag_start.y-y, x-drag_start.x);
            if(up) saveMap();
            break;
        case SET_GOAL:
            goal.x = drag_start.x / SCALE;
            goal.y = h- drag_start.y / SCALE;
            goal.theta = std::atan2(drag_start.y-y, x-drag_start.x);
            //            searchAlgorithm.setGoal(goal);
            if(up) saveMap();
            break;
        case SET_OBSTACLE:
            selection = cv::Rect(drag_start, cv::Point(x, y));
            break;
        case MAKE_MAZE:
            selection = cv::Rect(drag_start, cv::Point(x, drag_start.y + 4));
            break;
        default:
            refresh = false;
            break;
        }
    }

    x_ = x;
    y_ = y;

    if(refresh) {
        refresh_requested_ = refresh;
    }
}

template <class Search>
Evaluator<Search>::Evaluator(int w, int h, double resolution)
    : map_info(w, h, resolution), lastest_path_(NULL), show_info_(true), arrow_color(uni_tuebingen::cd::primary::karmin_red),
      complete_repaint_(false), refresh_requested_(false),
      drag(DRAG_NONE), drag_mode(SET_FOCUS), w(w), h(h), circle_radius(-1), res(resolution),
      first_or_last_frame(false), save_video_frames(false), force_no_wait(false), image_(1)
{
    obstacles = true;

    window = "Path Algorithm Evaluator";
    file_ = "map";

    cv::namedWindow(window);

    if(!loadMap()) {
        initDefaultMap();
    }

//    worker = boost::thread(boost::bind(&Evaluator::drawThread, this));

    img = cv::Mat(map_info.getHeight() * SCALE, map_info.getWidth() * SCALE, CV_8UC3, cv::Scalar::all(127));
}

template <class Search>
Evaluator<Search>::~Evaluator()
{
    worker.interrupt();
    worker.join();
}


template <class Search>
void Evaluator<Search>::setFocus(int x, int y)
{
    circle_radius = hypot(x-drag_start.x, y - drag_start.y) / 2.0;
    circle_center = 0.5 * (drag_start + cv::Point(x,y));
}

template <class Search>
void Evaluator<Search>::initDefaultMap()
{
    start.x = 1;
    start.y = 1;
    start.theta = M_PI / 2;

    //goal.x = w / 2 + 20;
    goal.x = w - 1;
    goal.y = h - 1;
    goal.theta = 0*M_PI;

    map_info.setOrigin(Point2d(0, 0));
    map_info.setResolution(res);
    map_info.setLowerThreshold(20);
    map_info.setUpperThreshold(50);

    orig_map = cv::Mat (h, w, CV_8UC1, cv::Scalar::all(255));

    generateMap();
}

template <class Search>
void Evaluator<Search>::screenshot()
{
    cv::imwrite("screenshot.png", img);
    std::cout << "wrote screenshot" << std::endl;
}

template <class Search>
void Evaluator<Search>::invertMap(bool save)
{

    for(int y=0; y<h; y++) {
        for(int x=0; x<w; x++) {
            uint8_t& val = orig_map.at<uint8_t>(y, x);
            val = 255 - val;
        }
    }
    generateMap(save);
}

template <class Search>
void Evaluator<Search>::clearMap(bool save)
{
    cv::rectangle(orig_map, cv::Rect(0,0,img.cols, img.rows), cv::Scalar::all(255), -1);
    generateMap(save);
}

namespace {
template <class Pt>
class Maze {
public:
    Maze(int width, cv::Mat orig_map)
        : width(std::max(3, width)), orig_map(orig_map), border(1)
    {
        w = orig_map.rows / width;
        h = orig_map.cols / width;
        N = w * h;

        border = std::max(1, width/4);

        init();
    }

    void run(const Pt& start, const Pt& goal) {
        cv::Point s(start.x / width, start.y / width);
        cv::Point g(goal.x / width, goal.y / width);

        expanded = 0;

        std::stack<cv::Point> open;
        cv::Point current = s;
        mark(current);

        while(expanded < N) {
            if(hasUnvisitedNeighbor(current)) {
                cv::Point next = getRandomUnvisitedNeighborCell(current);
                assert(!visited[IDX(next.x, next.y)]);

                open.push(current);

                removeWallsBetween(current, next);

                current = next;
                mark(current);

                //                cv::imshow("debug", orig_map);
                //                cv::waitKey(25);

            } else if(!open.empty()) {
                current = open.top();
                open.pop();

            } else {
                std::cout << "find random" << std::endl;
                current = getRandomUnvisitedCell();
                mark(current);
            }
        }
        std::cout << "done" << std::endl;

        assert(expanded == N);
    }

    ~Maze() {
        delete[] visited;
    }

private:
    void init() {
        memset(dx__, 0, 4 * sizeof(int));
        memset(dy__, 0, 4 * sizeof(int));
        dx__[0] = -1; dx__[2] = 1;
        dy__[1] = -1; dy__[3] = 1;

        visited = new int[N];
        memset(visited, true, (N) * sizeof(int));

        for(int y=0; y<w; y++) {
            for(int x=0; x<h; x++) {
                visited[IDX(x,y)] = false;
            }
        }
    }

    void mark(const cv::Point& pt) {
        visited[IDX(pt.x, pt.y)] = true;
        expanded++;
    }

    bool hasUnvisitedNeighbor(const cv::Point& pt) {
        for(int n = 0; n < 4; ++n) {
            int idx = IDX(pt.x + dx__[n], pt.y+dy__[n]);
            if(idx >= 0 && !visited[idx]) {
                return true;
            }
        }

        return false;
    }

    cv::Point getRandomUnvisitedCell() {
        while(true) {
            int x = rand() % h;
            int y = rand() % w;

            while(visited[IDX(x, y)]) {
                ++x;
                if(x == h) {
                    x = 0;
                    ++y;
                }
                if(y == w) {
                    y = 0;
                }
            }

            return cv::Point(x,y);
        }

        assert(false);
    }

    cv::Point getRandomUnvisitedNeighborCell(const cv::Point& pt) {
        assert(hasUnvisitedNeighbor(pt));

        while(true) {
            int n = rand() % 4;
            int dx = dx__[n];
            int dy = dy__[n];

            int idx = IDX(pt.x + dx, pt.y+dy);
            if(idx >= 0 && !visited[idx]) {
                cv::Point next(pt.x + dx, pt.y + dy);
                if(next.x < 0 || next.y < 0 || next.x == h || next.y == w) {
                    continue;
                } else {
                    return next;
                }
            }
        }

        assert(false);
    }

    void removeWallsBetween(const cv::Point& current, const cv::Point& next) {
        cv::Rect r1 = rectFor(current);
        cv::Rect r2 = rectFor(next);
        cv::Rect middle(0.5 * (r1.tl() + r2.tl()), r1.size());

        cv::rectangle(orig_map, r1, cv::Scalar::all(255), CV_FILLED);
        cv::rectangle(orig_map, middle, cv::Scalar::all(255), CV_FILLED);
        cv::rectangle(orig_map, r2, cv::Scalar::all(255), CV_FILLED);
    }

    inline int IDX(int x, int y) {
        if(x < 0 || y < 0 || x == h || y == w) {
            return -1;
        }
        return y*h + x;
    }

    inline void free(const cv::Point& pt) {
        free(pt.x, pt.y);
    }

    inline void free(int x, int y) {
        cv::rectangle(orig_map, rectFor(x, y), cv::Scalar::all(255), CV_FILLED);
    }

    inline cv::Rect rectFor(const cv::Point& pt) {
        return rectFor(pt.x, pt.y);
    }

    inline cv::Rect rectFor(int x, int y){
        return cv::Rect(x*width+border/2,y*width+border/2,width-border,width-border);
    }

private:
    int dx__[4];
    int dy__[4];

    int width;
    cv::Mat orig_map;
    int border;

    int N;
    int w;
    int h;

    int* visited;
    int expanded;
};
}

template <class Search>
void Evaluator<Search>::makeMaze(int width)
{
    clearMap(false);
    invertMap(false);

    Maze<Pose2d> m(std::max(1, width / SCALE), orig_map);
    m.run(start, goal);

    generateMap(true);
}

template <class Search>
void Evaluator<Search>::generateMap(bool save)
{
    for(int y=0; y<h; y++) {
        for(int x=0; x<w; x++) {
            uint8_t val = orig_map.at<uint8_t>(y, x);

            if(val == 0) {
                /// OBSTACLE
                map_info.setValue(x, y, 255);
                assert(!map_info.isFree(x, y));
            } else {
                map_info.setValue(x, y, 0);
                assert(map_info.isFree(x, y));
            }
        }
    }

    if(save) {
        saveMap();
    }
}

template <class Search>
void Evaluator<Search>::saveMap()
{
    cv::imwrite(file_ + ".ppm", orig_map);

    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "resolution" << YAML::Value << res;
    out << YAML::Key << "start" << YAML::Value << YAML::BeginSeq << start.x << start.y << start.theta << YAML::EndSeq;
    out << YAML::Key << "goal" << YAML::Value << YAML::BeginSeq << goal.x << goal.y << goal.theta << YAML::EndSeq;

    out << YAML::EndMap;

    std::ofstream f_out((file_ + ".yaml").c_str());
    f_out << out.c_str();

    std::cout << "saved map to \"" << file_ << "\"" << std::endl;
}

template <class Search>
bool Evaluator<Search>::loadMap()
{
    orig_map = cv::imread(file_ + ".ppm", 0);

    if(orig_map.empty()){
        return false;
    }

    if(w != orig_map.cols || h != orig_map.rows ) {
        return false;
    }

    w = orig_map.cols;
    h = orig_map.rows;

    std::ifstream fin((file_ + ".yaml").c_str());
    YAML::Parser parser(fin);

    YAML::Node doc;
    if(!parser.GetNextDocument(doc)) {
        return false;
    }

    double res_tmp;
    doc["resolution"] >> res_tmp;

    if(res_tmp != res) {
        return false;
    }

    res = res_tmp;

    doc["start"][0] >> start.x;
    doc["start"][1] >> start.y;
    doc["start"][2] >> start.theta;

    doc["goal"][0] >> goal.x;
    doc["goal"][1] >> goal.y;
    doc["goal"][2] >> goal.theta;

    map_info.setOrigin(Point2d(0, 0));
    map_info.setResolution(res);
    map_info.setLowerThreshold(20);
    map_info.setUpperThreshold(50);

    generateMap(false);

    std::cout << "loaded map from \"" << file_ << "\"" << std::endl;

    return true;
}

template <class Search>
typename Evaluator<Search>::KEY_EVENT Evaluator<Search>::wait()
{

    int key = cv::waitKey(20) & 0xFF;
    KEY_EVENT res = keyCallback(key);

    if(res == KEY_EVENT_EXIT || cvGetWindowHandle(window.c_str()) == NULL) {
        exit(0);
    }

    return res;
}

template <class Search>
void Evaluator<Search>::drawThread()
{
    while(!boost::this_thread::interruption_requested()) {
        boost::unique_lock<boost::mutex> lock(mut);
        while(!draw_cond_state) {
            draw_cond.wait(lock);
        }

        draw(true);
        draw_cond_state = false;
    }
}

template <class Search>
void Evaluator<Search>::drawInParallel()
{
    if(!draw_cond_state)
    {
        boost::lock_guard<boost::mutex> lock(mut);
        draw_cond_state = true;
    }
    draw_cond.notify_one();
}

template <class Search>
void Evaluator<Search>::draw(bool use_wait)
{
    boost::lock_guard<boost::recursive_mutex> lock(mut_paint);

    algo.setOut(img);
    algo.renderMap();

    algo.visualizeSearchSpace();

    if(lastest_path_ != NULL) {
        algo.renderPath(*lastest_path_);
    }

    if(show_info_)
    {
        std::stringstream ss;
        ss << "expansions: " << algo.noExpansions() << ", ";
        ss << "multi expansions: " << algo.noMultiExpansions() << ", ";
        ss << "updated nodes: " << algo.noUpdates();
        cv::putText(img, ss.str(), cv::Point(40, 40), cv::FONT_HERSHEY_PLAIN, 0.7, cv::Scalar::all(255), 3, CV_AA);
        cv::putText(img, ss.str(), cv::Point(40, 40), cv::FONT_HERSHEY_PLAIN, 0.7, cv::Scalar::all(0), 1, CV_AA);

        ss.str(std::string());
        ss << "mode: ";
        ss << "(F, default) " << ((drag_mode == SET_FOCUS) ? "SET FOCUS" : "set focus");
        ss << "  ";
        ss << "(S) " << ((drag_mode == SET_START) ? "SET START" : "set start");
        ss << "  ";
        ss << "(G) " << ((drag_mode == SET_GOAL) ? "SET GOAL" : "set goal");
        ss << "  ";
        ss << "(o) " << ((drag_mode == SET_OBSTACLE) ? "SET OBSTACLE" : "set obstacle");
        ss << "  ";
        ss << "(t) toggle this menu";
        cv::putText(img, ss.str(), cv::Point(40, 80), cv::FONT_HERSHEY_PLAIN, 0.7, cv::Scalar::all(255), 3, CV_AA);
        cv::putText(img, ss.str(), cv::Point(40, 80), cv::FONT_HERSHEY_PLAIN, 0.7, cv::Scalar::all(0), 1, CV_AA);
    }

    if(selection.width > 0) {
        cv::rectangle(img, selection, cv::Scalar::all(0), 1, 8, 0);
    }
    algo.renderArrow(start, arrow_color);
    algo.renderArrow(goal, arrow_color);

    cv::imshow(window.c_str(), img);

    if(save_video_frames) {
        if(image_ == 1){
            first_or_last_frame = true;
        }
        image_++;

        if(first_or_last_frame) {
            int fps = video_.fps();
            for(int i = 0; i < 3 * fps; ++i) {
                video_ << img;
            }
            first_or_last_frame = false;
        } else {
            video_ << img;
        }
    }

    if(!use_wait || force_no_wait) {
        return;
    }

    wait();
}

template <class Search>
typename Evaluator<Search>::KEY_EVENT Evaluator<Search>::keyCallback(int key)
{
    char ckey = key;

    if(ckey == '-') {
        algo.render_factor_mult(0.75);

    } else if(ckey == '+') {
        algo.render_factor_mult(1.25);

    } else if(key == 'z') {
        algo.render_offset_add(0.1);

    } else if(key == 'h') {
        algo.render_offset_add(-0.1);

    } else if(key == 's') {
        drag_mode = SET_START;

    } else if(key == 'g') {
        drag_mode = SET_GOAL;

    } else if(key == 'f') {
        drag_mode = SET_FOCUS;

    } else if(key == 'm') {
        drag_mode = MAKE_MAZE;

    } else if(key == 'o') {
        drag_mode = SET_OBSTACLE;

    } else if(key == 'c') {
        clearMap();

    } else if(key == 'i') {
        invertMap();

    } else if(key == 'p') {
        screenshot();

    } else if(key == 't') {
        show_info_ = !show_info_;

    } else if(key == ' ') {
        return KEY_EVENT_SKIP;
    } else if(key == 27) {
        return KEY_EVENT_EXIT;
    } else if(key != 255) {
        std::cout << "unbound key " << key << " pressed" << std::endl;
        return KEY_EVENT_NONE;
    } else if(key == 255) {
        return KEY_EVENT_NONE;
    }

    complete_repaint_ = true;

    return KEY_EVENT_ANY;
}

template <class Search>
bool Evaluator<Search>::run(bool skip_init, bool loop)
{
    algo.setMap(&map_info);
    algo.setStart(start);
    algo.setGoal(goal);

    init<Heuristic>(generic::Int2Type<HeuristicMapTraits<Heuristic>::HeuristicUsesInitFunction>());

    cv::setMouseCallback(window.c_str(), mouse_cb, this);

    if(!skip_init) {
        paintLoop();
    }

    std::cout << "start search" << std::endl;

    Stopwatch watch;
//    SearchAlgorithm::PathT path = algo.findPath(start, goal, boost::bind(&Evaluator::drawInParallel, this));
    PathT path = algo.findPath(start, goal, boost::bind(&Evaluator<Search>::draw, this, true));
    std::cout << "path search took " << watch.usElapsed() / 1e3 << "ms" << std::endl;

    lastest_path_ = &path;
    first_or_last_frame = true;
    draw(true);
    lastest_path_ = NULL;

    if(loop) {
        paintLoop(&path);
    }
}

template <class Search>
void Evaluator<Search>::render(const std::string& file)
{
    save_video_frames = true;
    show_info_ = false;
//    force_no_wait = true;

    video_.open(file + ".avi", 24, cv::Size(w, h), 25000);

    if (!video_.isOpened()) {
        std::cout  << "Could not open the output video for write: " << file << std::endl;
        return;
    }

    run(true, false);
}

template <class Search>
bool Evaluator<Search>::paintLoop(PathT* path)
{
    complete_repaint_ = true;

    while(cvGetWindowHandle(window.c_str()) != NULL) {

        if(complete_repaint_) {
            boost::lock_guard<boost::recursive_mutex> lock(mut_paint);

            complete_repaint_ = false;
            algo.setOut(img);
            draw();

            if(path != NULL) {
                algo.renderPath(*path);
            }
        }

        cv::Mat canvas;
        {
            boost::lock_guard<boost::recursive_mutex> lock(mut_paint);
            img.copyTo(canvas);
            algo.setOut(canvas);

            if(selection.width > 0) {
                cv::rectangle(canvas, selection, cv::Scalar::all(0), 1, 8, 0);
                cv::rectangle(canvas, selection+cv::Point(1,1), cv::Scalar::all(255), 1, 8, 0);
            }
            if(circle_radius > 0) {
                cv::circle(canvas, circle_center, circle_radius, cv::Scalar::all(0), 1, 8);
            }

            algo.renderArrow(start, arrow_color);
            algo.renderArrow(goal, arrow_color);
        }

        while(cvGetWindowHandle(window.c_str()) != NULL) {
            cv::imshow(window.c_str(), canvas);
            KEY_EVENT res = wait();

            if(refresh_requested_){
                refresh_requested_ = false;
                break;
            }
            if(res == KEY_EVENT_ANY) {
                break;
            } else if(res == KEY_EVENT_EXIT) {
                return false;
            } else if(res == KEY_EVENT_SKIP) {
                return true;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    int w = 600;
    int h = 400;
    double res = 0.1;

    if(argc > 1) {
        std::stringstream ss;
        for(int i=1; i < argc; ++i) {
            ss << argv[i] << " ";
        }
        YAML::Parser parser(ss);

        YAML::Node doc;
        if(!parser.GetNextDocument(doc)) {
            std::cout << "cannot read input \"" << ss.str() << "\"" << std::endl;
            return 1;
        }

#define READ(name) if(doc.FindValue(name)) doc[name]

        READ("w") >> w;
        READ("h") >> h;
        READ("res") >> res;

#undef READ

        std::cout << "w=" << w << ", h=" << h << ", res=" << res << std::endl;
    }

//        typedef search_algorithms::BFS Search;
//        typedef search_algorithms::Dijkstra Search;
    typedef search_algorithms::AStar Search;
//        typedef search_algorithms::AStarTaxi Search;
//        typedef search_algorithms::Dijkstra4d Search;
//    typedef search_algorithms::AStarMax Search;
//        typedef search_algorithms::DTA Search;
//            typedef search_algorithms::AStarNHHH Search;
//    typedef search_algorithms::AStarNH Search;
//    typedef search_algorithms::AStarNHOverEstimate Search;
//        typedef search_algorithms::AStarNHNoEndOrientation Search;

#define RENDER(T) { \
    Evaluator<search_algorithms::T> eval##T(w / Evaluator<search_algorithms::T>::SCALE, h / Evaluator<search_algorithms::T>::SCALE, res); \
    eval##T.render(#T); }

//    RENDER(DTA);
//    RENDER(BFS);
//    RENDER(Dijkstra);
//    RENDER(Dijkstra4d);
//    RENDER(AStar);
//    RENDER(AStarTaxi);
//    RENDER(AStarMax);
//    RENDER(AStarNH);
//    RENDER(AStarNHHH);
//    RENDER(AStarNHNoEndOrientation);

    Evaluator<Search> eval(w / Evaluator<Search>::SCALE, h / Evaluator<Search>::SCALE, res); \
    bool run = eval.run();
    while(run) {
        run = eval.run(true);
    }
}
