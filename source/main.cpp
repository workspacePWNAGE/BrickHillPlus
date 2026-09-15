#include <wx/wx.h>
#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;

static bool ReadIniValue(const std::string &iniPath, const std::string &key, std::string &value)
{
    std::ifstream in(iniPath);
    if (!in.is_open())
        return false;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.rfind(key + "=", 0) == 0)
        {
            value = line.substr(key.length() + 1);
            value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
            return true;
        }
    }
    return false;
}

static void UpdateIniKeys(const std::string &iniPath, const std::vector<std::pair<std::string, std::string>> &values)
{
    wxArrayString lines;
    std::vector<bool> found(values.size(), false);

    std::ifstream in(iniPath);
    if (in.is_open())
    {
        std::string line;
        while (std::getline(in, line))
        {
            bool matched = false;
            for (size_t i = 0; i < values.size(); ++i)
            {
                if (line.rfind(values[i].first + "=", 0) == 0)
                {
                    lines.Add(wxString(values[i].first + "=" + values[i].second));
                    found[i] = true;
                    matched = true;
                    break;
                }
            }
            if (!matched)
                lines.Add(wxString(line));
        }
        in.close();
    }

    for (size_t i = 0; i < values.size(); ++i)
    {
        if (!found[i])
            lines.Add(wxString(values[i].first + "=" + values[i].second));
    }

    std::ofstream out(iniPath, std::ios::trunc);
    if (out.is_open())
    {
        for (size_t i = 0; i < lines.GetCount(); ++i)
        {
            out << lines[i].ToStdString() << "\n";
        }
        out.close();
    }
}

struct BodyPart
{
    std::string iniKey;
    wxString label;
    wxRect rect;
    wxColour color;
};

class FigurePanel : public wxPanel
{
public:
    FigurePanel(wxWindow *parent)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
          m_hoveredIndex(-1)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        InitParts();
        LoadConfig();

        Bind(wxEVT_PAINT, &FigurePanel::OnPaint, this);
        Bind(wxEVT_MOTION, &FigurePanel::OnMouseMove, this);
        Bind(wxEVT_LEAVE_WINDOW, &FigurePanel::OnMouseLeave, this);
        Bind(wxEVT_LEFT_DOWN, &FigurePanel::OnLeftDown, this);
    }

private:
    std::vector<BodyPart> m_parts;
    int m_hoveredIndex;

    void InitParts()
    {
        m_parts = {
            {"HeadColor", "Head", wxRect(148, 33, 84, 84), wxColour(255, 165, 0)},
            {"LeftArmColor", "L. Arm", wxRect(86, 117, 52, 120), wxColour(255, 165, 0)},
            {"TorsoColor", "Torso", wxRect(138, 117, 104, 120), wxColour(204, 204, 204)},
            {"RightArmColor", "R. Arm", wxRect(242, 117, 52, 120), wxColour(255, 165, 0)},
            {"LeftLegColor", "L. Leg", wxRect(138, 237, 52, 120), wxColour(102, 102, 102)},
            {"RightLegColor", "R. Leg", wxRect(190, 237, 52, 120), wxColour(102, 102, 102)}
        };
    }

    void LoadConfig()
    {
        for (auto &part : m_parts)
        {
            std::string iniVal;
            if (ReadIniValue("./Content/Client/Player.ini", part.iniKey, iniVal) && !iniVal.empty())
            {
                std::string hex = (iniVal[0] == '#') ? iniVal : ("#" + iniVal);
                if (hex.length() == 7)
                    part.color = wxColour(hex);
            }
        }
    }

    void SaveConfig()
    {
        auto toHexNoHash = [](const wxColour &c) {
            return wxString::Format("%02X%02X%02X", c.Red(), c.Green(), c.Blue()).ToStdString();
        };

        fs::create_directories("./Content/Client");
        std::vector<std::pair<std::string, std::string>> iniValues;
        for (const auto &part : m_parts)
        {
            iniValues.push_back({part.iniKey, toHexNoHash(part.color)});
        }
        UpdateIniKeys("./Content/Client/Player.ini", iniValues);
    }

    void OnPaint(wxPaintEvent &event)
    {
        wxAutoBufferedPaintDC pdc(this);
        wxGCDC dc(pdc);

        dc.SetBackground(wxBrush(wxColour(30, 30, 30)));
        dc.Clear();

        for (int i = 0; i < (int)m_parts.size(); ++i)
        {
            const auto &part = m_parts[i];
            bool isHovered = (i == m_hoveredIndex);

            dc.SetBrush(wxBrush(part.color));
            if (isHovered)
            {
                dc.SetPen(wxPen(wxColour(255, 255, 255), 3));
            }
            else
            {
                dc.SetPen(wxPen(wxColour(20, 20, 20), 2));
            }
            dc.DrawRectangle(part.rect);

            dc.SetBrush(wxBrush(part.color));
            if (isHovered)
            {
                dc.SetPen(wxPen(wxColour(255, 255, 255), 3));
            }
            else
            {
                dc.SetPen(wxPen(wxColour(20, 20, 20), 2));
            }
            dc.DrawRectangle(part.rect);

            dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
            
            int brightness = (part.color.Red() * 299 + part.color.Green() * 587 + part.color.Blue() * 114) / 1000;
            dc.SetTextForeground(brightness > 128 ? wxColour(20, 20, 20) : wxColour(240, 240, 240));
        }
    }

    void OnMouseMove(wxMouseEvent &event)
    {
        wxPoint pos = event.GetPosition();
        int oldHovered = m_hoveredIndex;
        m_hoveredIndex = -1;

        for (int i = 0; i < (int)m_parts.size(); ++i)
        {
            if (m_parts[i].rect.Contains(pos))
            {
                m_hoveredIndex = i;
                break;
            }
        }

        if (m_hoveredIndex != oldHovered)
        {
            SetCursor(m_hoveredIndex != -1 ? wxCURSOR_HAND : wxCURSOR_DEFAULT);
            Refresh(false);
        }
    }

    void OnMouseLeave(wxMouseEvent &event)
    {
        if (m_hoveredIndex != -1)
        {
            m_hoveredIndex = -1;
            SetCursor(wxCURSOR_DEFAULT);
            Refresh(false);
        }
    }

    void OnLeftDown(wxMouseEvent &event)
    {
        wxPoint pos = event.GetPosition();
        for (auto &part : m_parts)
        {
            if (part.rect.Contains(pos))
            {
                wxColourData data;
                data.SetColour(part.color);
                data.SetChooseFull(true);

                wxColourDialog dialog(this, &data);
                dialog.SetTitle("Select " + part.label + " Color");

                if (dialog.ShowModal() == wxID_OK)
                {
                    part.color = dialog.GetColourData().GetColour();
                    SaveConfig();
                    Refresh(false);
                }
                break;
            }
        }
    }
};

class FigureDialog : public wxDialog
{
public:
    FigureDialog(wxWindow *parent)
        : wxDialog(parent, wxID_ANY, "Figure Customizer", wxDefaultPosition, wxSize(390, 460),
                   wxDEFAULT_DIALOG_STYLE & ~wxRESIZE_BORDER)
    {
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        
        wxStaticText *infoText = new wxStaticText(this, wxID_ANY, "Click any body part to edit its color:", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
        infoText->SetForegroundColour(wxColour(200, 200, 200));
        infoText->SetBackgroundColour(wxColour(30, 30, 30));
        infoText->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        
        FigurePanel *panel = new FigurePanel(this);

        sizer->Add(infoText, 0, wxEXPAND | wxTOP | wxBOTTOM, 10);
        sizer->Add(panel, 1, wxEXPAND);

        SetBackgroundColour(wxColour(30, 30, 30));
        SetSizer(sizer);
        Layout();
    }
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString &title);

private:
    wxTextCtrl *m_txtUsername;
    wxTextCtrl *m_txtServerIP;
    wxTextCtrl *m_txtServerPort;
    wxButton *m_btnHost;
    wxButton *m_btnJoin;
    wxButton *m_btnWorkshop;
    wxButton *m_btnFigure;

    bool m_serverRunning;
    long m_serverPid;

    void OnHostServer(wxCommandEvent &event);
    void OnJoinServer(wxCommandEvent &event);
    void OnWorkshop(wxCommandEvent &event);
    void OnFigure(wxCommandEvent &event);
    void OnWindowClose(wxCloseEvent &event);

    std::string SelectMap();
    void WriteStartJs(const std::string &ip, const std::string &port, const std::string &map);
    void UpdatePlayerIni(const std::string &iniPath, const std::string &cleanName);
    void SaveUsername(const std::string &username);
    std::string LoadUsername();
    void OpenLogTerminal();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();
    MyFrame *frame = new MyFrame("Brick Hill+");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString &title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400, 310),
              wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)),
      m_serverRunning(false), m_serverPid(0)
{
    this->Bind(wxEVT_CLOSE_WINDOW, &MyFrame::OnWindowClose, this);

    wxIcon appIcon;
    if (appIcon.LoadFile("Content/Assets/Icon.png", wxBITMAP_TYPE_PNG))
    {
        SetIcon(appIcon);
    }

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer *gridSizer = new wxFlexGridSizer(3, 2, 10, 10);
    gridSizer->AddGrowableCol(1, 1);

    wxStaticText *lblUsername = new wxStaticText(this, wxID_ANY, "Username:");
    std::string savedUsername = LoadUsername();
    m_txtUsername = new wxTextCtrl(this, wxID_ANY, savedUsername);
    gridSizer->Add(lblUsername, 0, wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(m_txtUsername, 1, wxEXPAND);

    wxStaticText *lblServerIP = new wxStaticText(this, wxID_ANY, "Server IP:");
    m_txtServerIP = new wxTextCtrl(this, wxID_ANY, "127.0.0.1");
    gridSizer->Add(lblServerIP, 0, wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(m_txtServerIP, 1, wxEXPAND);

    wxStaticText *lblServerPort = new wxStaticText(this, wxID_ANY, "Server Port:");
    m_txtServerPort = new wxTextCtrl(this, wxID_ANY, "42480");
    gridSizer->Add(lblServerPort, 0, wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(m_txtServerPort, 1, wxEXPAND);

    mainSizer->Add(gridSizer, 0, wxEXPAND | wxALL, 15);

    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxBoxSizer *leftColumnSizer = new wxBoxSizer(wxVERTICAL);
    m_btnHost = new wxButton(this, wxID_ANY, "Host Server");
    m_btnWorkshop = new wxButton(this, wxID_ANY, "Open Workshop");
    leftColumnSizer->Add(m_btnHost, 0, wxEXPAND | wxBOTTOM, 5);
    leftColumnSizer->Add(m_btnWorkshop, 0, wxEXPAND);

    wxBoxSizer *rightColumnSizer = new wxBoxSizer(wxVERTICAL);
    m_btnJoin = new wxButton(this, wxID_ANY, "Join Server");
    m_btnFigure = new wxButton(this, wxID_ANY, "Open Figure Editor");
    rightColumnSizer->Add(m_btnJoin, 0, wxEXPAND | wxBOTTOM, 5);
    rightColumnSizer->Add(m_btnFigure, 0, wxEXPAND);

    buttonSizer->Add(leftColumnSizer, 1, wxRIGHT, 5);
    buttonSizer->Add(rightColumnSizer, 1, wxLEFT, 5);
    
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 15);

    m_btnHost->Bind(wxEVT_BUTTON, &MyFrame::OnHostServer, this);
    m_btnJoin->Bind(wxEVT_BUTTON, &MyFrame::OnJoinServer, this);
    m_btnWorkshop->Bind(wxEVT_BUTTON, &MyFrame::OnWorkshop, this);
    m_btnFigure->Bind(wxEVT_BUTTON, &MyFrame::OnFigure, this);

    SetSizer(mainSizer);
    Layout();

    CreateStatusBar();
    SetStatusText("Ready");
}

std::string MyFrame::SelectMap()
{
    std::string mapsDir = "./Content/NodeHill/maps";
    if (!fs::exists(mapsDir))
    {
        return "";
    }

    wxArrayString mapChoices;
    for (const auto &entry : fs::directory_iterator(mapsDir))
    {
        if (entry.is_regular_file())
        {
            mapChoices.Add(wxString(entry.path().filename().string()));
        }
    }
    if (mapChoices.IsEmpty())
        return "";

    wxSingleChoiceDialog dialog(this, "Select a map to host:", "Map Selection", mapChoices);
    if (dialog.ShowModal() == wxID_OK)
    {
        return dialog.GetStringSelection().ToStdString();
    }
    return "";
}


void MyFrame::WriteStartJs(const std::string &ip, const std::string &port, const std::string &map)
{
    std::ofstream out("./Content/NodeHill/startold.js");
    out << "const nh = require('node-hill');\n"
        << "nh.startServer({\n"
        << "    hostKey: '',\n"
        << "    gameId: 478,\n"
        << "    port: " << port << ",\n"
        << "    ip: \"" << ip << "\",\n"
        << "    local: true,\n"
        << "    mapDirectory: './maps/',\n"
        << "    map: '" << map << "',\n"
        << "    scripts: './user_scripts',\n"
        << "    modules: [\"fs\"]\n"
        << "});\n";
    out.close();
}

void MyFrame::UpdatePlayerIni(const std::string &iniPath, const std::string &cleanName)
{
    wxArrayString lines;
    bool found = false;
    std::ifstream in(iniPath);
    if (in.is_open())
    {
        std::string line;
        while (std::getline(in, line))
        {
            if (line.rfind("Username=", 0) == 0)
            {
                lines.Add(wxString("Username=" + cleanName));
                found = true;
            }
            else
            {
                lines.Add(wxString(line));
            }
        }
        in.close();
    }
    if (!found)
    {
        lines.Add(wxString("Username=" + cleanName));
    }
    std::ofstream out(iniPath, std::ios::trunc);
    if (out.is_open())
    {
        for (size_t i = 0; i < lines.GetCount(); ++i)
        {
            out << lines[i].ToStdString() << "\n";
        }
        out.close();
    }
}

void MyFrame::SaveUsername(const std::string &username)
{
    std::string cleanName = username;
    cleanName.erase(std::remove(cleanName.begin(), cleanName.end(), '"'), cleanName.end());
    cleanName.erase(std::remove(cleanName.begin(), cleanName.end(), '\n'), cleanName.end());
    cleanName.erase(std::remove(cleanName.begin(), cleanName.end(), '\r'), cleanName.end());
    if (cleanName.empty())
        cleanName = "Player";

    fs::create_directories("./Content/Client");
    UpdatePlayerIni("./Content/Client/Player.ini", cleanName);
}

std::string MyFrame::LoadUsername()
{
    std::ifstream in("./Content/Client/Player.ini");
    if (in.is_open())
    {
        std::string line;
        while (std::getline(in, line))
        {
            if (line.rfind("Username=", 0) == 0)
            {
                std::string username = line.substr(9);
                username.erase(std::remove(username.begin(), username.end(), '\r'), username.end());
                username.erase(std::remove(username.begin(), username.end(), '\n'), username.end());
                
                if (!username.empty())
                {
                    return username;
                }
            }
        }
    }
    return "Guest";
}

void MyFrame::OpenLogTerminal()
{
    wxArrayString terminals;
    terminals.Add("kitty --title \"Brick Hill+ Server Logs\" tail -f ./Content/NodeHill/server.log");
    terminals.Add("alacritty --title \"Brick Hill+ Server Logs\" -e tail -f ./Content/NodeHill/server.log");
    terminals.Add("konsole --title \"Brick Hill+ Server Logs\" -e tail -f ./Content/NodeHill/server.log");
    terminals.Add("gnome-terminal --title=\"Brick Hill+ Server Logs\" -- tail -f ./Content/NodeHill/server.log");
    terminals.Add("xfce4-terminal --title=\"Brick Hill+ Server Logs\" -e \"tail -f ./Content/NodeHill/server.log\"");

    for (size_t i = 0; i < terminals.GetCount(); ++i)
    {
        std::string termCmd = terminals[i].ToStdString();
        std::size_t spacePos = termCmd.find(' ');
        std::string exeName = (spacePos != std::string::npos) ? termCmd.substr(0, spacePos) : termCmd;
        std::string checkCmd = "which " + exeName + " > /dev/null 2>&1";
        if (system(checkCmd.c_str()) == 0)
        {
            wxExecute(wxString(termCmd), wxEXEC_ASYNC);
            return;
        }
    }

    wxExecute("/bin/sh -c \"xterm -title \\\"Brick Hill+ Server Logs\\\" -e tail -f ./Content/NodeHill/server.log\"", wxEXEC_ASYNC);
}

void MyFrame::OnHostServer(wxCommandEvent &event)
{
    if (m_serverRunning)
    {
        if (m_serverPid > 0)
        {
            wxKill(m_serverPid, wxSIGTERM);
        }
        wxExecute("pkill -f startold.js", wxEXEC_ASYNC | wxEXEC_NODISABLE);
        m_serverRunning = false;
        m_serverPid = 0;
        m_btnHost->SetLabel("Host Server");
        SetStatusText("Server stopped.");
    }
    else
    {
        std::string ip = m_txtServerIP->GetValue().ToStdString();
        std::string port = m_txtServerPort->GetValue().ToStdString();
        std::string chosenMap = SelectMap();
        if (chosenMap.empty())
        {
            SetStatusText("Hosting cancelled or no maps found!");
            return;
        }

        fs::create_directories("./Content/NodeHill");
        if (!fs::exists("./Content/NodeHill/package.json"))
        {
            wxExecute("/bin/sh -c \"cd ./Content/NodeHill && npm init -y\"", wxEXEC_SYNC);
        }
        if (!fs::exists("./Content/NodeHill/node_modules/node-hill"))
        {
            SetStatusText("Installing node-hill package...");
            wxSafeYield();
            wxExecute("/bin/sh -c \"cd ./Content/NodeHill && npm install node-hill\"", wxEXEC_SYNC);
        }

        WriteStartJs(ip, port, chosenMap);

        std::ofstream clearLog("./Content/NodeHill/server.log", std::ios::trunc);
        clearLog.close();

        std::string cmd = "/bin/sh -c \"cd ./Content/NodeHill && node startold.js > server.log 2>&1\"";
        m_serverPid = wxExecute(wxString(cmd), wxEXEC_ASYNC);

        if (m_serverPid > 0)
        {
            m_serverRunning = true;
            m_btnHost->SetLabel("Stop Server");
            SetStatusText("Server running on " + ip + ":" + port + " (" + chosenMap + ")");
            OpenLogTerminal();
        }
        else
        {
            SetStatusText("Failed to initialize Node server.");
        }
    }
}

void MyFrame::OnJoinServer(wxCommandEvent &event)
{
    std::string username = m_txtUsername->GetValue().ToStdString();
    std::string ip = m_txtServerIP->GetValue().ToStdString();
    std::string port = m_txtServerPort->GetValue().ToStdString();
    if (!fs::exists("./Content/Client/Client.exe"))
    {
        SetStatusText("Error: ./Content/Client/Client.exe not found!");
        return;
    }
    SaveUsername(username);

    std::string cmd = "/bin/sh -c \"GAMEID=0 umu-run Content/Client/Client.exe \\\"" + ip + "/" + ip + "/" + port + "\\\"\"";
    wxExecute(wxString(cmd), wxEXEC_ASYNC);
    SetStatusText("Joining server as " + username + "...");
}

void MyFrame::OnWorkshop(wxCommandEvent &event)
{
    std::string workshopPath = "./Content/BrickBuilder/BrickBuilder.x86_64";
    if (!fs::exists(workshopPath))
    {
        SetStatusText("Error: " + workshopPath + " not found!");
        return;
    }

    std::string cmd = "/bin/sh -c \"" + workshopPath + "\"";
    wxExecute(wxString(cmd), wxEXEC_ASYNC);
    SetStatusText("Opening Workshop...");
}

void MyFrame::OnFigure(wxCommandEvent &event)
{
    FigureDialog dlg(this);
    dlg.ShowModal();
    SetStatusText("Updated figure colors.");
}

void MyFrame::OnWindowClose(wxCloseEvent &event)
{
    if (m_serverRunning && m_serverPid > 0)
    {
        wxKill(m_serverPid, wxSIGTERM);
    }
    
    wxExecute("pkill -f startold.js", wxEXEC_SYNC);
    event.Skip();
}