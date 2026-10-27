#include "calculator.h"
#include "ui_calculator.h"

#include <QKeyEvent>
#include <QPushButton>

namespace {

// 按钮 objectName -> 统一按键码，鼠标点击与键盘输入共用。
QString buttonKey(const QString &objectName)
{
    if (objectName == QLatin1String("btnDot"))
        return QStringLiteral(".");
    if (objectName == QLatin1String("btnPlus"))
        return QStringLiteral("+");
    if (objectName == QLatin1String("btnMinus"))
        return QStringLiteral("-");
    if (objectName == QLatin1String("btnMultiply"))
        return QStringLiteral("*");
    if (objectName == QLatin1String("btnDivide"))
        return QStringLiteral("/");
    if (objectName == QLatin1String("btnEquals"))
        return QStringLiteral("=");
    if (objectName == QLatin1String("btnClear"))
        return QStringLiteral("C");
    if (objectName == QLatin1String("btnBackspace"))
        return QStringLiteral("BACK");
    if (objectName.startsWith(QLatin1String("btn")) && objectName.size() == 4)
        return objectName.right(1); // btn0 ~ btn9
    return QString();
}

QString buttonRole(const QString &key)
{
    if (key.size() == 1 && key.at(0).isDigit())
        return QStringLiteral("digit");
    if (key == QLatin1String("."))
        return QStringLiteral("digit");
    if (key == QLatin1String("+") || key == QLatin1String("-")
        || key == QLatin1String("*") || key == QLatin1String("/"))
        return QStringLiteral("operator");
    if (key == QLatin1String("="))
        return QStringLiteral("equals");
    return QStringLiteral("function");
}

} // namespace

Calculator::Calculator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calculator)
{
    ui->setupUi(this);
    // 让自定义 QWidget 子类也能应用 style.qss 中的背景色。
    setAttribute(Qt::WA_StyledBackground, true);
    setFocusPolicy(Qt::StrongFocus);

    const auto buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        const QString key = buttonKey(button->objectName());
        if (key.isEmpty())
            continue;
        // 按钮不接收焦点，避免点击后按 Enter/空格误触发该按钮。
        button->setFocusPolicy(Qt::NoFocus);
        button->setProperty("role", buttonRole(key));
        connect(button, &QPushButton::clicked, this, [this, key] { handleKey(key); });
    }

    // U+232B 退格符号只在符号字体中存在，单独指定字体避免显示成方框。
    ui->btnBackspace->setFont(QFont(QStringLiteral("Segoe UI Symbol")));

    resetCalculator();
    setFocus();
}

Calculator::~Calculator()
{
    delete ui;
}

void Calculator::handleKey(const QString &key)
{
    if (key.isEmpty())
        return;

    // 除零错误后：先复位，只有数字或小数点能开始新的计算。
    if (m_error) {
        resetCalculator();
        const bool startsNumber = (key.size() == 1 && key.at(0).isDigit())
                                  || key == QLatin1String(".");
        if (!startsNumber)
            return;
    }

    if (key.size() == 1 && key.at(0).isDigit()) {
        inputDigit(key);
    } else if (key == QLatin1String(".")) {
        inputDecimalPoint();
    } else if (key == QLatin1String("+") || key == QLatin1String("-")
               || key == QLatin1String("*") || key == QLatin1String("/")) {
        inputOperator(key);
    } else if (key == QLatin1String("=")) {
        calculateResult();
    } else if (key == QLatin1String("C")) {
        resetCalculator();
    } else if (key == QLatin1String("BACK")) {
        backspace();
    }
}

void Calculator::inputDigit(const QString &digit)
{
    if (m_resultShown)
        resetCalculator(); // 结果后直接输入数字，开始新的计算

    if (m_waitingForOperand) {
        m_currentText = digit;
        m_waitingForOperand = false;
    } else if (m_currentText == QLatin1String("0")) {
        m_currentText = digit; // 避免 "05" 这样的前导零
    } else {
        m_currentText += digit;
    }

    m_expression = m_pendingOperator.isEmpty()
                       ? QString()
                       : m_firstText + QLatin1Char(' ') + operatorSymbol(m_pendingOperator);
    refreshDisplay();
}

void Calculator::inputDecimalPoint()
{
    if (m_resultShown)
        resetCalculator();

    if (m_waitingForOperand) {
        m_currentText = QStringLiteral("0.");
        m_waitingForOperand = false;
    } else if (!m_currentText.contains(QLatin1Char('.'))) {
        // 同一操作数只允许一个小数点，重复按下直接忽略。
        m_currentText += QLatin1Char('.');
    } else {
        return;
    }

    m_expression = m_pendingOperator.isEmpty()
                       ? QString()
                       : m_firstText + QLatin1Char(' ') + operatorSymbol(m_pendingOperator);
    refreshDisplay();
}

void Calculator::inputOperator(const QString &op)
{
    if (m_resultShown) {
        // 结果后按运算符：以结果为第一个操作数继续计算。
        m_resultShown = false;
        m_firstOperand = currentValue();
        m_firstText = m_currentText;
    } else if (m_pendingOperator.isEmpty()) {
        m_firstOperand = currentValue();
        m_firstText = m_currentText;
    } else if (m_waitingForOperand) {
        // 连续按运算符：只替换运算符，不重复计算。
        m_pendingOperator = op;
        m_expression = m_firstText + QLatin1Char(' ') + operatorSymbol(op);
        refreshDisplay();
        return;
    } else {
        bool ok = true;
        const double result = applyOperator(m_firstOperand, currentValue(), m_pendingOperator, &ok);
        if (!ok) {
            showDivisionByZeroError(m_firstText + QLatin1Char(' ')
                                    + operatorSymbol(m_pendingOperator) + QLatin1Char(' ')
                                    + m_currentText);
            return;
        }
        m_firstOperand = result;
        m_firstText = formatNumber(result);
        m_currentText = m_firstText;
    }

    m_pendingOperator = op;
    m_waitingForOperand = true;
    m_expression = m_firstText + QLatin1Char(' ') + operatorSymbol(op);
    refreshDisplay();
}

void Calculator::calculateResult()
{
    if (m_pendingOperator.isEmpty())
        return; // 没有待执行的运算，忽略 "="

    // 例如 "5 + =" 时第二个操作数取第一个操作数，与常见计算器一致。
    const double secondOperand = m_waitingForOperand ? m_firstOperand : currentValue();
    const QString secondText = m_waitingForOperand ? m_firstText : m_currentText;
    const QString expression = m_firstText + QLatin1Char(' ')
                               + operatorSymbol(m_pendingOperator) + QLatin1Char(' ')
                               + secondText + QStringLiteral(" =");

    bool ok = true;
    const double result = applyOperator(m_firstOperand, secondOperand, m_pendingOperator, &ok);
    if (!ok) {
        showDivisionByZeroError(expression);
        return;
    }

    m_currentText = formatNumber(result);
    m_firstOperand = result;
    m_firstText = m_currentText;
    m_expression = expression;
    m_pendingOperator.clear();
    m_waitingForOperand = false;
    m_resultShown = true;
    refreshDisplay();
}

void Calculator::backspace()
{
    // 结果状态或等待第二个操作数时退格无意义，直接忽略。
    if (m_resultShown || m_waitingForOperand)
        return;

    if (m_currentText.size() <= 1)
        m_currentText = QStringLiteral("0");
    else
        m_currentText.chop(1);

    m_expression = m_pendingOperator.isEmpty()
                       ? QString()
                       : m_firstText + QLatin1Char(' ') + operatorSymbol(m_pendingOperator);
    refreshDisplay();
}

void Calculator::resetCalculator()
{
    m_firstOperand = 0.0;
    m_firstText.clear();
    m_pendingOperator.clear();
    m_currentText = QStringLiteral("0");
    m_expression.clear();
    m_waitingForOperand = false;
    m_resultShown = false;
    m_error = false;
    refreshDisplay();
}

void Calculator::showDivisionByZeroError(const QString &expression)
{
    m_error = true;
    m_currentText = QStringLiteral("除数不能为0");
    m_expression = expression;
    m_pendingOperator.clear();
    m_firstText.clear();
    m_waitingForOperand = false;
    m_resultShown = false;
    refreshDisplay();
}

double Calculator::applyOperator(double lhs, double rhs, const QString &op, bool *ok) const
{
    *ok = true;
    if (op == QLatin1String("+"))
        return lhs + rhs;
    if (op == QLatin1String("-"))
        return lhs - rhs;
    if (op == QLatin1String("*"))
        return lhs * rhs;
    if (op == QLatin1String("/")) {
        if (qFuzzyIsNull(rhs)) {
            *ok = false;
            return 0.0;
        }
        return lhs / rhs;
    }
    *ok = false;
    return 0.0;
}

double Calculator::currentValue() const
{
    return m_currentText.toDouble();
}

QString Calculator::formatNumber(double value) const
{
    if (value == 0.0)
        value = 0.0; // 把 -0 归一化成 0
    return QString::number(value, 'g', 15);
}

QString Calculator::operatorSymbol(const QString &op) const
{
    if (op == QLatin1String("*"))
        return QStringLiteral("×");
    if (op == QLatin1String("/"))
        return QStringLiteral("÷");
    if (op == QLatin1String("-"))
        return QStringLiteral("−");
    return op;
}

QString Calculator::keyForEvent(QKeyEvent *event)
{
    const int key = event->key();
    if (key >= Qt::Key_0 && key <= Qt::Key_9)
        return QString::number(key - Qt::Key_0);

    switch (key) {
    case Qt::Key_Plus:
        return QStringLiteral("+");
    case Qt::Key_Minus:
        return QStringLiteral("-");
    case Qt::Key_Asterisk:
        return QStringLiteral("*");
    case Qt::Key_Slash:
        return QStringLiteral("/");
    case Qt::Key_Period:
    case Qt::Key_Comma:
        return QStringLiteral(".");
    case Qt::Key_Equal:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return QStringLiteral("=");
    case Qt::Key_Backspace:
        return QStringLiteral("BACK");
    case Qt::Key_Escape:
    case Qt::Key_Delete:
        return QStringLiteral("C");
    default:
        break;
    }

    // 小键盘小数点在不同键盘布局下可能以文本形式给出。
    const QString text = event->text();
    if (text == QLatin1String(".") || text == QLatin1String(","))
        return QStringLiteral(".");
    return QString();
}

void Calculator::keyPressEvent(QKeyEvent *event)
{
    const QString key = keyForEvent(event);
    if (key.isEmpty()) {
        QWidget::keyPressEvent(event);
        return;
    }
    handleKey(key);
    event->accept();
}

void Calculator::refreshDisplay()
{
    ui->expressionLabel->setText(m_expression);
    ui->displayLabel->setText(m_currentText);
}
