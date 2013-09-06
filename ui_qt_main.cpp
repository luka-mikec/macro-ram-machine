#include "ui_qt_main.h"
#include "ui_ui_qt_main.h"
#include "ui_qt_dialog.h"
#include "QMessageBox"
#include <sstream>
#include <fstream>

using namespace std;

typedef int number;
typedef int adress;

struct macro;

map<adress, number> mem;
map<adress, string> vars;
map<string, macro>  macro_lib;

string sourcecode;

struct command
{
    enum { _inc, _dec, _goto, _stop } act;
    adress n = 0, k = 0; // params

    string act_to_str()
    {
        if (act == _inc) return "inc";
        if (act == _dec) return "dec";
        if (act == _goto) return "goto";
        if (act == _stop) return "stop";
    }
};

struct macro
{
    vector<command> metacode;
    int argc;

    vector<command> inst(int line, vector<int> args, map<adress, adress> &macro_to_actual)
    {
        vector<command> obj;
        for (command comm : metacode)
        {
            if (comm.n < 0)
                if (comm.act == command::_goto)
                    if (args.at(-comm.n - 1) >= 0)
                        comm.n = macro_to_actual[args.at(-comm.n - 1)];
                    else
                        comm.n = args.at(-comm.n - 1);
                else
                    comm.n = args.at(-comm.n - 1);
            else if (comm.act == command::_goto)
                comm.n += line;

            if (comm.k < 0)
                if (comm.act == command::_dec)
                    if (args.at(-comm.k - 1) >= 0)
                        comm.k = macro_to_actual[args.at(-comm.k - 1)];
                    else
                        comm.k = args.at(-comm.k - 1);
                else
                    comm.k = args.at(-comm.k - 1);
            else if (comm.act == command::_dec)
                    comm.k += line;

            obj.push_back(comm);
        }
        return obj;
    }
};

vector <command> bytecode;

void inject_macro(stringstream& in, string act, map<adress, adress> &macro_to_actual)
{
    auto w = macro_lib.find(act);
    if (w == macro_lib.end()) return;
    else {
        vector <int> args; int argc = w->second.argc;
        while (argc--) {args.push_back(0); in >> args.back();}
        auto obj = w->second.inst(bytecode.size(), args, macro_to_actual);
        bytecode.insert(bytecode.end(), obj.begin(), obj.end());
    }
}

void compile()
{
    stringstream in;
    bytecode.clear();

    map<adress, adress> macro_to_actual;
    vector<adress> mark_points;
    stringstream prerun; prerun << sourcecode;
    adress cadress = 0, i = 0;
    bool modified = false;
    while (prerun)
    {
        macro_to_actual[i++] = cadress;
        string w; prerun >> w;
        string tmp; int n = 0; // n = how many items do skip
        if (w == "mark")
        {
            prerun >> w;
            QString nsourcecode = QString::fromStdString(sourcecode);
            QString nw = QString::fromStdString(w);
            nsourcecode.replace(nw, QString::number(i - 1));
            cadress += 2; // nop instruction
            sourcecode = nsourcecode.toStdString();
            modified = true;
        }
        else if (macro_lib.find(w) != macro_lib.end())
        {
            auto &mac = macro_lib[w];
            cadress += mac.metacode.size();
            n = mac.argc;
        }
        else
        {
            cadress++;
            if (w == "inc" || w == "goto")
                n = 1;
            else if (w == "dec")
                n = 2;
        }
        while (n--) {prerun >> tmp;
            if (tmp[0] == ':')
            {
                QString nsourcecode = QString::fromStdString(sourcecode);
                QString nw = QString::fromStdString(tmp);
                nsourcecode.replace(nw, QString::number(100000 + 100 * i + n));
                sourcecode = nsourcecode.toStdString();
                vars[100000 + 100 * i + n] = tmp;
            }
        }
    }

    in << sourcecode;
    /*if (modified) alert("", sourcecode, 0);*/

    while (in)
    {
        command newcomm;
        string act; in >> act;
        if (act == "inc") newcomm.act = command::_inc;
        else if (act == "dec") newcomm.act = command::_dec;
        else if (act == "goto") newcomm.act = command::_goto;
        else if (act == "stop") newcomm.act = command::_stop;
        else if (act != "") {
            inject_macro(in, act, macro_to_actual);
            continue;
        } else break;

        if (newcomm.act < command::_stop) in >> newcomm.n;
        if (newcomm.act == command::_dec) in >> newcomm.k;

        // update lines, unless it's a meta thing
        if (newcomm.act == command::_goto && newcomm.n >= 0)
            newcomm.n = macro_to_actual[newcomm.n];
        if (newcomm.act == command::_dec && newcomm.k >= 0)
            newcomm.k = macro_to_actual[newcomm.k];

        bytecode.push_back(newcomm);
    }
}

adress command_pointer;
bool halt;

void step()
{
    command &comm = bytecode[command_pointer];
    if (comm.act == command::_stop) halt = true;
    else if (comm.act == command::_goto) command_pointer = comm.n;
    else if (comm.act == command::_inc) {
        mem[comm.n]++;
        ++command_pointer;
    } else if (comm.act == command::_dec) {
        if (mem[comm.n]) {--mem[comm.n]; ++command_pointer; }
        else command_pointer = comm.k;
    }
}

void boot(QWidget *parent_wnd = 0)
{
    command_pointer = 0;
    halt = false;
    mem.clear();

    adress cmdcnt = 0;
    while (command_pointer < bytecode.size() && !halt)
    {
        step();
        ++cmdcnt;
        if (cmdcnt % 20000000 == 0)
        {
            if (QMessageBox::question(parent_wnd, "inf loop interceptor",
                                          "you might be headed for infinity, stop?")
                == QMessageBox::Yes)
                break;
        }
    }
}

void dump(Ui::MainWindow *ui)
{
    ui->listWidget->clear();
    if (mem.size())
    for (auto &v : mem)
        if ((ui->checkBox->checkState() == Qt::Checked) ||
                (v.first % 1000000 != 0 || v.first == 0))
        ui->listWidget->addItem(
                    QString::fromStdString(vars.find(v.first) != vars.end() ? vars[v.first] + "/" : "") +
                    QString::number(v.first, 10) + " : " +
                    QString::number(v.second, 10));
}

void push_macro(string name, int argc, string source)
{
    macro _;
    sourcecode = source;
    compile();
    _.argc = argc; _.metacode = bytecode;
    macro_lib[name] = _;
}

void init_macros()
{
    // -1 += 'i'
    string cache = ""; for (int i = 1; i <= 128; ++i)
    push_macro("inc" + QString::number(i).toStdString(), 1, cache += " inc -1");

    fstream libloader("./stdlib.mac");
    if (!libloader) return;

    string libsrc; string mac_name = ""; int mac_arity = 0;
    string w; while (libloader >> w)
    {
        if (w == "//") { libloader.ignore(1000000, '\n'); }
        else if (w == "new") {
            if (mac_name != "") push_macro(mac_name, mac_arity, libsrc);
            libsrc = ""; libloader >> mac_name >> mac_arity;
        }
        else libsrc += w + " ";
    }


}



void ui_compile(Ui::MainWindow *ui)
{
    sourcecode = ui->plainTextEdit->toPlainText().toStdString();
    compile();
}

void ui_run(Ui::MainWindow *ui, QWidget *parent_wnd = 0)
{
    boot(parent_wnd);
    dump(ui);
}

void alert(string ttl, string msg, QWidget *parent_wnd, bool fullblown)
{
    if (fullblown)
    {
        ui_qt_dialog* d = new ui_qt_dialog(parent_wnd, msg);
        d->exec();
        delete d;
    }
    else
    QMessageBox::about(parent_wnd, QString::fromStdString(ttl), QString::fromStdString(msg));
}

string bytecode_to_str(vector<command> &bc)
{
    int n = 0; stringstream fullcode;
    for (command c : bc)
        fullcode << "\t" << n++ << " " << c.act_to_str() << " " << c.n << " " << c.k << endl;
    return fullcode.str();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init_macros();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_triggered()
{
    ui->plainTextEdit->clear();
}

void MainWindow::on_actionLoad_triggered()
{
    QMessageBox::about(this, "done", "lolol not implemented");
}

void MainWindow::on_actionInspect_stdlib_triggered()
{
    string outp = "";
    fstream libloader("./stdlib.mac");
    if (libloader)
    {
        outp = "tip: this window is resizeable\nstdlib Macro-RAM code (scroll down for actual RAM code):\n\n";
        char c;
        while ((c = libloader.get()) != -1)
            outp += c;
    }

    outp += "\n\n\nRAM (compiled) code:\n\n incN -1\n\tinc -1\n\t...\n\tinc -1\n\n";
    for (auto &_m : macro_lib)
    {
        if (_m.first.size() >= 3)
            if (_m.first.substr(0, 3) == "inc")
                continue;
        macro &m = _m.second;
        outp += _m.first + "\n" + bytecode_to_str(m.metacode) + "\n";
    }
    alert("macro_lib", outp, this);

}

void MainWindow::on_actionExit_triggered()
{
    exit(0);
}

void MainWindow::on_actionCompile_triggered()
{
    ui_compile(ui);
    alert("compiler", "full code:\n" + bytecode_to_str(bytecode), this);
}

void MainWindow::on_actionRun_triggered()
{
    ui_run(ui, this);
}

void MainWindow::on_actionCompile_Run_triggered()
{
    ui_compile(ui);
    ui_run(ui, this);
}

void MainWindow::on_pushButton_clicked()
{
    //ui->listWidget->addItem("lol");
    ui_compile(ui);
    ui_run(ui, this);
}

void MainWindow::on_pushButton_2_clicked()
{
    ui_compile(ui);
    alert("compiler", "full code:\n" + bytecode_to_str(bytecode), this);
}

void MainWindow::on_actionAbout_triggered()
{
    alert("", "Luka Mikec, luka dot mikec1 at gmail dot com:\n2013\n\n"
          "Go to github.com/luka-mikec for more info.", this);
}

void MainWindow::on_pushButton_3_clicked()
{
    ui_run(ui, this);
}
