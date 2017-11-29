/*--------------------------------------------------------------------------------------------------------------------*/
/*
 * main.cpp
 *
 * Copyright © 2017 Yuto Mizutani.
 * This software is released under the MIT License.
 *
 * Version: 1.0.0
 *
 * Author: Yuto Mizutani
 * E-mail: yuto.mizutani.dev@gmail.com
 * Website: http://operantroom.com
 *
 * Created: 2017/11/27
 * Device: MacBook Pro (Retina, 13-inch, Mid 2015)
 * OS: macOS Serria version 10.12.6
 * IDE: CLion 2017.2.3
 * JRE: 1.8.0_152-release-915-b12 x86_64
 *
 * > gcc -v
 * Configured with: --prefix=/Applications/Xcode.app/Contents/Developer/usr --with-gxx-include-dir=/usr/include/c++/4.2.1
 * Apple LLVM version 9.0.0 (clang-900.0.38)
 * Target: x86_64-apple-darwin16.7.0
 *
 * 初めてのc++。
 * 以前のc++歴はHello worldと時間計測程度。
 * X or shift という高速な乱数生成方法があるというので，乱数を利用したプログラムを作ってみようと思った。
 * 今月頭に始めたFGOのガチャシミュレータを作ることにした。某wikiのwebガチャツールがスクロールしないと再び引けないという仕様を見て決めた。
 * 200行くらいと予想したが，意外と長くなってしまった。
 * 日付を超えたので肝心の乱数生成は後回しに。
 * とりあえず動くところまで。
 *
 * 予定
 * ・ファイル分け
 * ・X or shiftの実装
 * ・ガチャクラス（protocol）の生成（他ガチャへの変更への対応）
 * ・メンバ変数アクセスを別メソッドにまとめる。
 * ・ガチャ結果周りをスマートにする。
 * ・WorkOnTerminalクラス，文字取得クラスとループクラスに分ける。
 * ・条件分岐のメソッドを移動させる。
 * ・どこか見にくさを感じるので考える。
 */
/*--------------------------------------------------------------------------------------------------------------------*/

#include <iostream>
#include <array>
#include <vector>
#include <random>

/*--------------------------------------------------------------------------------------------------------------------*/

template <typename T>
void print(T t, bool isLine=true) {
    if (isLine) {
        std::cout
                << t
                << std::endl;
    }else{
        std::cout
                << t
                << std::flush;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

// http://dx.doi.org/10.18637/jss.v008.i14
class RandomGenerator {
    unsigned int w, x, y, z;
public:
    RandomGenerator(unsigned int seed = 88675123U);
private:
    unsigned int getSeed();
    unsigned int xOrShift();
public:
    float getRandom(unsigned int max=100000);
    unsigned int getRandomRange(int min, int max);
};
RandomGenerator::RandomGenerator(unsigned int seed) : w(seed) {
    x = 123456789U;
    y = 362436069U;
    z = 521288629U;
}
unsigned int RandomGenerator::getSeed(){
    // create seed from current time
    auto n = std::chrono::high_resolution_clock::now();
    auto d = n - n.min();
    // https://stackoverflow.com/questions/12031302/convert-from-long-long-to-int-and-the-other-way-back-in-c
    // https://stackoverflow.com/questions/4975340/int-to-unsigned-int-conversion
    unsigned int s = (unsigned int) std::chrono::duration_cast<std::chrono::microseconds>(d).count();
    return s;
}
unsigned int RandomGenerator::xOrShift() {
    w = getSeed();
    unsigned int t = x ^ (x << 11);
    x = y;
    y = z;
    w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    return w;
}
unsigned int RandomGenerator::getRandomRange(int min, int max) {
    unsigned int value = xOrShift();
    return min + value % (max - min);
}
float RandomGenerator::getRandom(unsigned int max) {
    // 少数第3位まで

    // http://vivi.dyndns.org/tech/cpp/random.html
    std::random_device rnd;     // 非決定的な乱数生成器を生成
    std::mt19937 mt(rnd());     //  メルセンヌ・ツイスタの32ビット版、引数は初期シード値
    std::uniform_int_distribution<> rand100(0, max-1);        // [0, 99] 範囲の一様乱数
    float r = (float)rand100(mt)/1000.0f;

    return r;
}

/*--------------------------------------------------------------------------------------------------------------------*/

// http://vivi.dyndns.org/tech/cpp/timeMeasurement.html
class FGOGacha {
    // 乱数テーブル
    RandomGenerator m_randomizer;
    // 課金石名称
    std::string StoneName = "聖晶石";
    // ガチャ内容
    std::array<float, 100> m_prob; // 0.0~100.0(%)
    std::array<std::string, 100> m_prob_name;
    // ピックアップ状態
    bool m_isPickUp = false;
// MARK: FGOGacha::init
private:
    void setParameters();
public:
    FGOGacha();
    void changePickUpState(bool isPickUp);
    std::string getStoneName();
    unsigned int getStoneConsumption(unsigned int numOfGacha);
    unsigned int getStoneFee(unsigned int numOfStones);
// MARK: FGOGacha::public methods
public:
    std::vector<unsigned int> roll(unsigned int trials);
    std::array<std::string, 100> getProbName();
    unsigned int getArrayNum();
// MARK: FGOGacha::private methods
private:
    float getRandom(unsigned int max=100000);
    unsigned int rollOne(std::array<float, 100> prob);
    bool decisionPickRareCards(std::vector<unsigned int> history);
    bool decisionPickServant(std::vector<unsigned int> history);
    unsigned int reLotteryCraftEssence();
    unsigned int reLotteryServant();
    bool checkSumPercentage(std::array<float, 100> prob);
};

// MARK: FGOGacha::static method
void FGOGacha::setParameters() {
    // init a rarity percentage array ------
    std::fill(m_prob.begin(), m_prob.end(), 0);
    std::fill(m_prob_name.begin(), m_prob_name.end(), "");

    // set rarity m_probabilities ------
    if (m_isPickUp) {
        m_prob = {0.5, 0.5, 1.5, 1.5, 40.0,
                2.0, 2.0, 6.0, 6.0, 40.0};
        //          0      1      4       24      28     40     100(%)
        m_prob_name = {
                "鯖☆5(ﾋﾟｯｸｱｯﾌﾟ)",
                "鯖☆5         ",
                "鯖☆4(ﾋﾟｯｸｱｯﾌﾟ)",
                "鯖☆4         ",
                "鯖☆3         ",
                "礼☆5(ﾋﾟｯｸｱｯﾌﾟ)",
                "礼☆5         ",
                "礼☆4(ﾋﾟｯｸｱｯﾌﾟ)",
                "礼☆4         ",
                "礼☆3         "
        };
    }else{
        m_prob = {1.0, 3.0, 40.0,
                4.0, 12.0, 40.0};
        //          0      1      4       24      28     40     100(%)
        m_prob_name = {
                "鯖☆5",
                "鯖☆4",
                "鯖☆3",
                "礼☆5",
                "礼☆4",
                "礼☆3"
        };
    }
}
void FGOGacha::changePickUpState(const bool isPickUp) {
    m_isPickUp = isPickUp;
    setParameters();
    std::string s = (isPickUp ? "True" : "False");
    print("FGO ピックアップ状態が \""+s+"\"" + "に変更されました。");
}
std::string FGOGacha::getStoneName() {
    return StoneName;
}
unsigned int FGOGacha::getStoneConsumption(const unsigned int numOfGacha) {
    // https://game8.jp/fate-go/144558
    return numOfGacha*3;
}
unsigned int FGOGacha::getStoneFee(const unsigned int numOfStones) {
    // https://game8.jp/fate-go/144558
//    聖晶石の個数	金額	1個毎
//    167個	9,800円	58円
//    76個	4,800円	63円
//    41個	2,900円	70円
//    18個	1,400円	77円
//    5個	480円	96円
//    1個	120円	120円
    const unsigned int num = numOfStones;
    return (num/167)*9800 + ((num%167)/76)*4800 + (((num%167)%76)/41)*4800 +
           ((((num%167)%76)%41)/18)*1400 + (((((num%167)%76)%41)%18)/5)*480 + (((((num%167)%76)%41)%18)%5)*120;
}

// MARK: FGOGacha::init
FGOGacha::FGOGacha() {
    // init a randomizer
    m_randomizer = RandomGenerator();

    setParameters();
}

// MARK: FGOGacha::UseCases
std::vector<unsigned int> FGOGacha::roll(const unsigned int trials) {
    // variables
    std::vector<unsigned int> history;

    if (!checkSumPercentage(m_prob)) {
        std::cout << "No match gacha percentage!" << std::endl;
        return history;
    }
    if (trials == 0) {
        print("No trial!");
        return history;
    }

    print("\nTrials: "+std::to_string(trials));
    // https://stackoverflow.com/questions/34829955/what-is-causing-this-cannot-jump-from-switch-statement-to-this-case-label
    switch (trials) {
        case 10: {
            // case 10: 10連召喚
            for (unsigned int i = 0; i < (int)trials; ++i) {
                unsigned int r = rollOne(m_prob);
                history.push_back(r);
            }

            // 星4以上確定救済
            // 4以上が1つもなければ
            if (!decisionPickRareCards(history)) {
                // 再抽選の結果を格納
                unsigned int re = reLotteryCraftEssence();
                // 最初の結果を入れ替える
                history.at(0) = re;
            }
            // 星3鯖以上確定救済
            // 星3以上の鯖が1つもなければ
            if (!decisionPickServant(history)) {
                // 再抽選の結果を格納
                unsigned int re = reLotteryServant();
                // 最初の結果を入れ替える
                history.at(1) = re;
            }
            break;
        }
        default: {
            // case n: 1回(or 呼符)召喚 * trials
            for (unsigned int i = 0; i < (int) trials; ++i) {
                unsigned int r = rollOne(m_prob);
                history.push_back(r);
            }
            break;
        }
    }

    return history;
}
std::array<std::string, 100> FGOGacha::getProbName() {
    return m_prob_name;
};

// MARK: FGOGacha::Model
float FGOGacha::getRandom(const unsigned int max) {
    //小数第3位まで(ab.cde)
    return m_randomizer.getRandom();
}
bool FGOGacha::checkSumPercentage(std::array<float, 100> prob) {
    // ガチャの確率が100%かどうか判断する。
    float sum = 0;
    // http://vivi.dyndns.org/tech/cpp/range-for.html
    for (float x : prob) {
        sum += x;
    }
    return 100.0 == sum;
}
unsigned int FGOGacha::rollOne(std::array<float, 100> prob) {
    // ガチャを引く。返り値nは配列のn番目。

    // Get random value(0 to 100)
    float v = getRandom(100000);
    print(v);

    // check random value in range of array
    // 乱数がある確率の範囲内ならそれを返す。
    float sum = 0;
    // http://vivi.dyndns.org/tech/cpp/range-for.html
    for(unsigned int i = 0; i < (int)prob.size(); ++i) {
        sum += prob.at(i);
        if (v < sum) {
            // print result
            std::string s = "ガチャ結果: "+m_prob_name.at(i);
            print(s);

            return i;
        }
    }

    // if no matches
    std::cout << "Could not pick gacha contents!" << std::endl;
    return 0;
}
bool FGOGacha::decisionPickRareCards(std::vector<unsigned int> history) {
    // 全て☆3であったかを確認する。
    for (unsigned int i = 0; i != history.size(); ++i) {
        // 3の文字が見つかればtrueを返す。
        if ((int)m_prob_name.at(history.at(i)).find("☆3") == -1) {
            return true;
        }
    }
    return false;
}
bool FGOGacha::decisionPickServant(std::vector<unsigned int> history) {
    // 全て概念礼装であったかを確認する。
    for (unsigned int i = 0; i != history.size(); ++i) {
        // 3の文字が見つかればtrueを返す。
        if ((int)m_prob_name.at(history.at(i)).find("鯖") == -1) {
            return true;
        }
    }
    return false;
}
unsigned int FGOGacha::reLotteryCraftEssence(){
    // ☆4の再抽選。方法は
    // http://oudoon.blog.fc2.com/blog-entry-17.html
    // を参考に，
    // 礼装☆4が92.0%になるとする。
    print("10連救済措置：☆4再抽選!");

    std::array<float, 100> re_prob;
    if (m_isPickUp) {
        re_prob = {0.5, 0.5, 1.5, 1.5, 0.0,
                   2.0, 2.0, 46.0, 46.0, 0.0};
    }else{
        re_prob = {1.0, 3.0, 0.0,
                   4.0, 92.0, 0.0};
    }
    if (!checkSumPercentage(re_prob)) {
        std::cout << "[reLotteryCraftEssence] No match gacha percentage!" << std::endl;
        return 0;
    }
    return rollOne(re_prob);
}
unsigned int FGOGacha::reLotteryServant() {
    // ☆3鯖の再抽選。方法は
    // http://oudoon.blog.fc2.com/blog-entry-17.html
    // を参考に，
    // ☆3鯖が96.0%になるとする。
    print("10連救済措置：☆3鯖再抽選!");

    std::array<float, 100> re_prob;
    if (m_isPickUp) {
        re_prob = {0.5, 0.5, 1.5, 1.5, 96.0,
                   0.0, 0.0, 0.0, 0.0, 0.0};
    }else{
        re_prob = {1.0, 3.0, 96.0,
                   0.0, 0.0, 0.0};
    }
    if (!checkSumPercentage(re_prob)) {
        std::cout << "[reLotteryServant] No match gacha percentage!" << std::endl;
        return 0;
    }
    return rollOne(re_prob);
}
unsigned int FGOGacha::getArrayNum() {
    unsigned int r = 0;
    for (std::string x : m_prob_name) {
        if (x == "") {
            return r;
        }
        r += 1;
    }
    return r;
}

/*--------------------------------------------------------------------------------------------------------------------*/

class UserResult {
    // 課金額
    unsigned int m_cash = 0;
    // 所持石数
    unsigned int m_stones = 0;
    // ユーザー結果の保存
    std::vector<unsigned int> m_user_result;
    // Gacha class
    FGOGacha m_gacha;
public:
    UserResult(FGOGacha gacha);
public:
    void magicCard(unsigned int fee);
    std::string getUserParameterString();
    int computeGachable(unsigned int trials);

    void clearResult();

    int getUserTotalTrials();
    int getUserTotalFee();
    std::vector<unsigned int> cashing(std::vector<unsigned int> results);
    void showResult(std::vector<unsigned int> results);
};
UserResult::UserResult(FGOGacha gacha) {
    m_gacha = gacha;
    clearResult();
}

void UserResult::magicCard(unsigned int stones=167) {
    int fee = m_gacha.getStoneFee(stones);

    m_stones += stones;
    m_cash += fee;

    print("素晴らしい魔法のカードを使った！ - ¥", false);
    print(fee);

    getUserParameterString();
}
std::string UserResult::getUserParameterString() {
    std::string r = "";
    r += "累計課金額: "+std::to_string(m_cash)+"円";
    r += "\n";
    r += "所持"+m_gacha.getStoneName()+": "+std::to_string(m_stones);
    return r;
}
int UserResult::computeGachable(unsigned int trials) {
    // 残金-will試行ガチャ金額
    if (trials <= 0) {
        print("under 0 error");
        return -1;
    }

    int s = m_gacha.getStoneConsumption(trials);

    return m_stones - s;
}
void UserResult::clearResult() {
    // reset user results ------
    //      clear the vector
    m_user_result.clear();
    unsigned long c = m_gacha.getArrayNum();
    //      reinitialize the vector
    for (unsigned int i = 0; i < c; ++i) {
        // https://cpprefjp.github.io/reference/vector/push_back.html
        // https://ameblo.jp/woooh-p/entry-10040236926.html
        m_user_result.push_back(0);
    }
}

int UserResult::getUserTotalTrials() {
    int t = std::accumulate(m_user_result.begin(), m_user_result.end(), 0);
    return t;
}
int UserResult::getUserTotalFee() {
    int t = getUserTotalTrials();
    return (int)m_gacha.getStoneFee(m_gacha.getStoneConsumption(t));
}
std::vector<unsigned int> UserResult::cashing(std::vector<unsigned int> results) {
    auto t = (int)results.size(); //std::accumulate(results.begin(), results.end(), 0);
    int s = m_gacha.getStoneConsumption(t);

    int b = m_stones;

    // 消費
    m_stones -= s;

    print("残"+m_gacha.getStoneName()+": "+std::to_string(b)+"個 -> "+std::to_string(m_stones)+"個");
    return results;
}
void UserResult::showResult(std::vector<unsigned int> results) {
    // 結果を反映
    unsigned long n = m_gacha.getArrayNum();
    std::vector<unsigned int> r;
    for (unsigned int i = 0; i < n; ++i) {
        r.push_back(0);
    }
    for (unsigned int x : results) {
        r.at(x) += 1;
    }

    print("\n");
    print("----------------------------------------");
    print(std::to_string(results.size())+"連ガチャ結果", false);
    print(" (消費"+m_gacha.getStoneName()+": ", false);
    print(m_gacha.getStoneConsumption(results.size()), false);
    print("個, 金額: ", false);
    print((int)m_gacha.getStoneFee(m_gacha.getStoneConsumption(results.size())), false);
    print("円)", true);
    unsigned long c = m_gacha.getArrayNum();
    for (unsigned int i = 0; i < (int) c; ++i) {
        std::string m_rs = m_gacha.getProbName().at(i) + ": " + std::to_string(r.at(i));
        print(m_rs);
    }
    print("----------------------------------------");






    // ユーザー結果m_user_resultに反映
    for (unsigned int x : results) {
        m_user_result.at(x) += 1;
    }

    print("\n\n\n");
    print("----------------------------------------");
    print("ガチャ結果総まとめ", false);
    print(" (消費"+m_gacha.getStoneName()+": ", false);
    print(m_gacha.getStoneConsumption(getUserTotalTrials()), false);
    print("個, 金額: ", false);
    print(getUserTotalFee(), false);
    print("円)", true);
    c = m_gacha.getArrayNum();
    for (unsigned int i = 0; i < (int) c; ++i) {
        std::string m_rs = m_gacha.getProbName().at(i) + ": " + std::to_string(m_user_result.at(i));
        print(m_rs);
    }
    print("----------------------------------------");
}

class WorkOnTerminal {
    FGOGacha m_gacha = FGOGacha();
    UserResult m_user = UserResult(FGOGacha());
public:
    WorkOnTerminal();
    void setup();
    void loop();
public:
    std::string input();
    std::vector<std::string> split(std::string str, std::string separator);
};
WorkOnTerminal::WorkOnTerminal() {
    print("Gacha.cpp");
    print("Version: 1.0.0");
    print("Copyright © 2017 Yuto Mizutani.");
    print("This software is released under the MIT License.");
    print("");
}
void WorkOnTerminal::setup() {
    m_gacha = FGOGacha();
    m_gacha.changePickUpState(true);
    m_user = UserResult(m_gacha);
}
void WorkOnTerminal::loop() {
    bool isEnd = true;
    while (isEnd) {
        print("----------------------------------------");

        // Information
        print(m_user.getUserParameterString());
        print("\n");
        print("課金(石数): c, ガチャ: g, リセット: r, 終了: e, ヘルプ: h");
        std::string s = input();

        char c = s.front();
        switch (c) {
            case 'c': case 'C': {
                // 課金

                // 金額取得
                std::vector<std::string> rs = split(s, " ");
                std::string rn = rs.size() > 1 ? rs[1] : "0";
                auto n = (unsigned int)std::stoi(rn);

                m_user.magicCard(n<=0 ? 167 : n);
                continue;
            }
            case 'g': case 'G': case '\u0000': {
                // ガチャ

                // 回数取得
                std::vector<std::string> rs = split(s, " ");
                std::string rn = rs.size() > 1 ? rs[1] : "0";
                auto n = (unsigned int)std::stoi(rn);
                if (n <= 0) {
                    n = 10;
                }

                // 残高チェック
                int f = m_user.computeGachable(n);
                if (f < 0) {
                    print(m_gacha.getStoneName()+"が"+std::to_string(-f)+"個足りません。課金してください。");
                    continue;
                }

                // n連
                m_user.showResult(m_user.cashing(m_gacha.roll(n)));
                break;
            }
            case 'r': case 'R': {
                // 初期化
                print("ユーザ状態をリセットします。");
                setup();
                continue;
            }
            case 'e': case 'E': {
                // 終了
                isEnd = false;
                continue;
            }
            case 'h': case 'H': {
                print("[help]");
                print("c++で書かれたガチャシミュレータです。");
                print("コマンドで課金，ガチャを引くことができます。");
                print("コマンドの後にスペースと数値入力で，購入石数，ガチャ数を指定できます。");
                print("高速10連の機能もあり，コマンド無記入+Enterで10連ガチャをすぐに引くことができます。");
                print("c+Enterの後に，g+Enterをしてみてください。");
                continue;
            }
            default: {
                print("コマンドが設定されていません。");
                continue;
            }
        }
    }
}
std::string WorkOnTerminal::input() {
    std::string s;
    print(">> ", false);
    std::getline(std::cin, s);
//    print(s);
    return s;
}
std::vector<std::string> WorkOnTerminal::split(const std::string str, const std::string separator) {
    // http://marycore.jp/prog/cpp/std-string-split/
    auto separator_length = separator.length(); // 区切り文字の長さ

    auto list = std::vector<std::string>();

    if (separator_length == 0) {
        list.push_back(str);
    } else {
        auto offset = std::string::size_type(0);
        while (1) {
            auto pos = str.find(separator, offset);
            if (pos == std::string::npos) {
                list.push_back(str.substr(offset));
                break;
            }
            list.push_back(str.substr(offset, pos - offset));
            offset = pos + separator_length;
        }
    }

    return list;
}

/*--------------------------------------------------------------------------------------------------------------------*/

int main() {
    WorkOnTerminal terminal = WorkOnTerminal();
    terminal.setup();
    terminal.loop();

    return 0;
}
