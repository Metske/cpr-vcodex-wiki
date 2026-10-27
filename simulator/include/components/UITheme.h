#pragma once
struct Metrics {
    int topPadding = 0;
    int headerHeight = 20;
};
class UITheme {
public:
    static UITheme& getInstance() { static UITheme t; return t; }
    Metrics getMetrics() const { return Metrics{}; }
};
