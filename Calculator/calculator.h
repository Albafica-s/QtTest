#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Calculator;
}
QT_END_NAMESPACE

class QKeyEvent;

// 一个两操作数计算器：鼠标按键和键盘按键最终都调用 handleKey()，
// 保证两种输入方式共用同一套状态机。
class Calculator : public QWidget
{
    Q_OBJECT

public:
    explicit Calculator(QWidget *parent = nullptr);
    ~Calculator() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    // key 统一使用："0"~"9"、"."、"+"/"-"/"*"/"/"、"="、"C"、"BACK"。
    void handleKey(const QString &key);
    void inputDigit(const QString &digit);
    void inputDecimalPoint();
    void inputOperator(const QString &op);
    void calculateResult();
    void backspace();
    void resetCalculator();
    void showDivisionByZeroError(const QString &expression);

    double applyOperator(double lhs, double rhs, const QString &op, bool *ok) const;
    double currentValue() const;
    QString formatNumber(double value) const;
    QString operatorSymbol(const QString &op) const;
    static QString keyForEvent(QKeyEvent *event);
    void refreshDisplay();

    Ui::Calculator *ui;

    // 运算状态：第一个操作数、待执行的运算符、当前显示内容（第二个操作数或结果）。
    double m_firstOperand = 0.0;
    QString m_firstText;         // 第一个操作数的显示文本，用于表达式区
    QString m_pendingOperator;   // 空表示尚未选择运算符
    QString m_currentText = "0"; // 当前输入或计算结果
    QString m_expression;        // 显示在结果上方的表达式

    bool m_waitingForOperand = false; // 刚按下运算符，等待输入第二个操作数
    bool m_resultShown = false;       // 刚按下 "="，显示的是结果
    bool m_error = false;             // 处于除零错误状态
};

#endif // CALCULATOR_H
