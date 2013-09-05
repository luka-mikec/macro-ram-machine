#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include <sstream>

using namespace std;

typedef int number;
typedef int adress;

struct macro;

map<adress, number> mem;
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

    vector<command> inst(int line, vector<int> args, int macro_add)
    {
        vector<command> obj;
        for (command comm : metacode)
        {
            if (comm.n < 0)     comm.n = args.at(-comm.n - 1);
            else if (comm.act == command::_goto) comm.n += line;
            if (comm.k < 0)     comm.k = args.at(-comm.k - 1);
            else if (comm.act == command::_dec) comm.k += line;

         /*   if (comm.act == command::_goto)
                comm.n += macro_add;
            if (comm.act == command::_dec)
                comm.k += macro_add;*/

            obj.push_back(comm);
        }
        return obj;
    }
};

vector <command> bytecode;

void inject_macro(stringstream& in, string act, int macro_add)
{
    auto w = macro_lib.find(act);
    if (w == macro_lib.end()) return;
    else {
        vector <int> args; int argc = w->second.argc;
        while (argc--) {args.push_back(0); in >> args.back();}
        auto obj = w->second.inst(bytecode.size(), args, macro_add);
        bytecode.insert(bytecode.end(), obj.begin(), obj.end());
    }
}

void compile()
{
    stringstream in; in << sourcecode;
    bytecode.clear();

    map<adress, adress> macro_to_actual;
    stringstream prerun; prerun << sourcecode;
    adress cadress = 0, i = 0;
    while (prerun)
    {
        macro_to_actual[i++] = cadress;
        string w; prerun >> w;
        int tmp; int n = 0; // n = how many items do skip
        if (macro_lib.find(w) != macro_lib.end())
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
        while (n--) prerun >> tmp;
    }

    while (in)
    {
        command newcomm;
        string act; in >> act;
        if (act == "inc") newcomm.act = command::_inc;
        else if (act == "dec") newcomm.act = command::_dec;
        else if (act == "goto") newcomm.act = command::_goto;
        else if (act == "stop") newcomm.act = command::_stop;
        else if (act != "") {
            //int old = bytecode.size();
            inject_macro(in, act, 0);
            //macro_add += bytecode.size() - old - 1;
            continue;
        } else break;

        if (newcomm.act < command::_stop) in >> newcomm.n;
        if (newcomm.act == command::_dec) in >> newcomm.k;

        if (newcomm.act == command::_goto)
            newcomm.n = macro_to_actual[newcomm.n];
        if (newcomm.act == command::_dec)
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
        ui->listWidget->addItem(QString::number(v.first, 10) + " : " +
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

    // -1 := '0'
    push_macro("zero", 1,
               "dec -1 2"
               "goto 0"
               );
    // -1 += -2 using -3; -1 != -2
    push_macro("f_uadd_diff", 3,
               "zero -3 "
               "dec -2 5 "
               "inc -1 "
               "inc -3 "
               "goto 1"
               "dec -3 8 "
               "inc -2 "
               "goto 5 "
               );
    // -1 = -2 using -3; -1 != -2
    push_macro("f_set_diff", 3,
               "zero -1 "
               "f_uadd_diff -1 -2 -3 "
               );
    // -1 += -2 using -3, -4
    push_macro("f_uadd", 4,
               "f_set_diff -3 -2 -4 "
               "f_uadd_diff -1 -3 -4 "
               );
    // -1 = -2 using -3, -4
    push_macro("f_set", 4,
               "f_set_diff -3 -2 -4 "
               "zero -1 "
               "f_uadd_diff -1 -3 -4 "
               "zero -2 "
               "f_uadd_diff -2 -3 -4 "
               );
    // -1 += -2 using 1000000, 1000000
    push_macro("uadd", 2,
               "f_uadd -1 -2 1000000 2000000"
               );
    // -1 = -2 using 1000000, 2000000
    push_macro("set", 2,
               "f_set -1 -2 1000000 2000000"
               );
    // -1 -= -2 using -3, -4, -5
    push_macro("f_usub", 5,
               "f_set -3 -2 -4 -5 "
               "dec -3 4 "
               "dec -1 4 "
               "goto 1 "
               );
    // -1 -= -2 using 1000000, 2000000, 3000000
    push_macro("usub", 2,
               "f_usub -1 -2 1000000 2000000 3000000"
               );
    // -1 = -2 + -3, using -4, -5, -6, -7
    push_macro("f_add", 7,
               "f_set -4 -2 -6 -7 "
               "f_set -5 -3 -6 -7 "
               "zero -1 "
               "f_uadd -1 -4 -6 -7 "
               "f_uadd -1 -5 -6 -7 "
               );
    // -1 = -2 + -3, using 1000000, 2000000
    push_macro("add", 3,
               "f_add -1 -2 -3 1000000 2000000 3000000 4000000"
               );
    // -1 = -2 - -3, using -4, -5, -6, -7
    push_macro("f_sub", 7,
               "f_set -4 -2 -6 -7 "
               "f_usub -4 -3 -5 -6 -7 "
               "f_set -1 -4 -6 -7 "
               );
    // -1 = -2 - -3, using 1000000*: 1-4
    push_macro("sub", 3,
               "f_sub -1 -2 -3 1000000 2000000 3000000 4000000"
               );
    // -1 = -2 * -3, using -4 ... -7
    push_macro("f_mul", 7,
               "f_set -4 -2 -6 -7 "
               "zero -5 "
               "dec -4 5"
               "f_uadd -5 -3 -6 -7 "
               "goto 2 "
               "f_set -1 -5 -6 -7 ");
    // -1 = -2 * -3, using 1000000*: 1-4
    push_macro("mul", 3,
               "f_mul -1 -2 -3 1000000 2000000 3000000 4000000"
               );
    // -1 = sgn(-2)
    push_macro("sgn", 2,
               "dec -2 5 "
               "inc -2 "
               "zero -1 "
               "inc -1 "
               "goto 6 "
               "zero -1 "
               );
    // -1 = !-2
    push_macro("neg", 2,
               "sgn -1 -2 "
               "dec -1 3"
               "goto 4"
               "inc -1 "
               );
    // -1 = -2 && -3, using -4, -5, -6, -7
    push_macro("f_and", 7,
               "f_mul -1 -2 -3 -4 -5 -6 -7 "
               "sgn -1 -1 "
               );
    // -1 = -2 || -3, using -4, -5, -6, -7
    push_macro("f_or", 7,
               "f_add -1 -2 -3 -4 -5 -6 -7 "
               "sgn -1 -1 "
               );
    // -1 = -2 && -3, using e6 x4
    push_macro("and", 3,
               "f_and -1 -2 -3 1000000 2000000 3000000 4000000"
               );
    // -1 = -2 || -3, using e6 x4
    push_macro("or", 3,
               "f_or -1 -2 -3 1000000 2000000 3000000 4000000"
               );

    push_macro ("test", 0,
                "inc 2 "
                "inc 2 "
                "inc 5 "
                );
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
    QMessageBox::about(this, "done", "lolol");
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

void MainWindow::on_pushButton_clicked()
{
    //ui->listWidget->addItem("lol");
    ui_compile(ui);
    ui_run(ui, this);
}

void MainWindow::on_pushButton_2_clicked()
{
    ui_compile(ui);
    stringstream fullcode; fullcode << "full code:\n";
    int n = 0;
    for (command c : bytecode)
        fullcode << "\t" << n++ << " " << c.act_to_str() << " " << c.n << " " << c.k << endl;
    QMessageBox::about(this, "compiler", QString::fromStdString(fullcode.str()));
}

void MainWindow::on_pushButton_3_clicked()
{
    ui_run(ui, this);
}
