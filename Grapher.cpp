#include <windows.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>
#include "resource.h"

struct GraphData {
    std::vector<int> dataPoints;
    std::wstring graphType;
};

GraphData currentGraph = { {10, 50, 30, 80, 60, 110}, L"line" };

void DrawLineGraph(HDC hdc, const GraphData& graphData) {
    int numPoints = graphData.dataPoints.size();
    MoveToEx(hdc, 50, 150 - graphData.dataPoints[0], NULL);
    for (int i = 1; i < numPoints; ++i) {
        LineTo(hdc, 50 + i * 50, 150 - graphData.dataPoints[i]);
    }
}

void DrawBarChart(HDC hdc, const GraphData& graphData) {
    int numPoints = graphData.dataPoints.size();
    for (int i = 0; i < numPoints; ++i) {
        Rectangle(hdc, 50 + i * 50, 150, 80 + i * 50, 150 - graphData.dataPoints[i]);
    }
}

void DrawPieChart(HDC hdc, const GraphData& graphData) {
    int total = 0;
    for (int value : graphData.dataPoints) {
        total += value;
    }

    int startAngle = 0;
    int radius = 100;
    for (int value : graphData.dataPoints) {
        int sweepAngle = (value * 360) / total;
        Pie(hdc, 100, 100, 100 + radius * 2, 100 + radius * 2,
            100 + radius + radius * cos(startAngle * 3.14159 / 180),
            100 + radius - radius * sin(startAngle * 3.14159 / 180),
            100 + radius + radius * cos((startAngle + sweepAngle) * 3.14159 / 180),
            100 + radius - radius * sin((startAngle + sweepAngle) * 3.14159 / 180));
        startAngle += sweepAngle;
    }
}

void CreateMenus(HWND hwnd) {
    HMENU hMenubar = CreateMenu();
    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreateMenu();

    AppendMenu(hMenu, MF_STRING, 1, L"Line Graph");
    AppendMenu(hMenu, MF_STRING, 2, L"Bar Chart");
    AppendMenu(hMenu, MF_STRING, 3, L"Pie Chart");

    AppendMenu(hSubMenu, MF_STRING, 4, L"Customize Graph...");

    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"Graph Type");
    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hSubMenu, L"Options");
    SetMenu(hwnd, hMenubar);
}

INT_PTR CALLBACK CustomizeDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG: {
            HWND hComboBox = GetDlgItem(hDlg, IDC_COMBO_GRAPH_TYPE);
            SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Line Graph");
            SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Bar Chart");
            SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Pie Chart");
            return TRUE;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                wchar_t graphType[100];
                GetDlgItemText(hDlg, IDC_COMBO_GRAPH_TYPE, graphType, 100);
                
                currentGraph.graphType = std::wstring(graphType);

                wchar_t dataPoints[100];
                GetDlgItemText(hDlg, IDC_EDIT_DATA_POINTS, dataPoints, 100);

                std::wstringstream ss(dataPoints);
                std::vector<int> newPoints;
                int point;
                while (ss >> point) {
                    newPoints.push_back(point);
                    if (ss.peek() == ',') {
                        ss.ignore();
                    }
                }
                currentGraph.dataPoints = newPoints;

                EndDialog(hDlg, IDOK);
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateMenus(hwnd);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1:
            currentGraph.graphType = L"line";
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case 2:
            currentGraph.graphType = L"bar";
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case 3:
            currentGraph.graphType = L"pie";
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case 4:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GRAPH_CUSTOMIZE), hwnd, CustomizeDialogProc);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (currentGraph.graphType == L"line") {
            DrawLineGraph(hdc, currentGraph);
        }
        else if (currentGraph.graphType == L"bar") {
            DrawBarChart(hdc, currentGraph);
        }
        else if (currentGraph.graphType == L"pie") {
            DrawPieChart(hdc, currentGraph);
        }

        EndPaint(hwnd, &ps);
    } break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
    MSG msg;
    WNDCLASS wc = {0};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = _T("GraphingApp");

    if(!RegisterClass(&wc))
        return -1;

    HWND hwnd = CreateWindow(wc.lpszClassName, _T("Graphing Application"),
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             500, 500, NULL, NULL, hInstance, NULL);

    if(!hwnd)
        return -1;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}