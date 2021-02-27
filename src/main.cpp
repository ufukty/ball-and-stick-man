/*
2016 - github.com/ufukty
GPL-3.0 License
See the LICENSE file

-----------------------------------------------------
CONTROLS
-----------------------------------------------------

                        Forward
                           |
              Left arm     |     Right arm
                     |     |     |
    Left forearm     |     |     |     Right forarm      Camera up
               |     |     |     |     |                    |
            *-----*-----*-----*-----*-----*              *-----*
            |  Q  |  W  |  E  |  R  |  T  |              |  ▲  |
            *-----*-----*-----*-----*-----*        *-----*-----*-----*
                  |  S  |  D  |  F  |              |  ◀︎  |  ▼  |  ►  |
                  *-----*-----*-----*              *-----*-----*-----*
                     |     |     |                    |     |     |
                  Left     |      Right     Camera left     |     Camera right
                           |                                |
                       Backward                        Camera down

    ! Toggle the caps to reverse the effect.


-----------------------------------------------------
ANIMATIONS
-----------------------------------------------------
    Left-click mouse  : Switch between walking modes
    Right-click mouse : Toggle waving
*/

#include <iostream>
#include <vector>
#include <cmath>
#include <glut.h>

/////////////////////////////////////////////////////////////////// SABİTLER

// sin ve cos fonksiyonlarında radyan dönüşümü için
#define PI 3.1415926535

// Her Object nesnesinin alacağı şekli belirtmek için

#define SPHERE 0
#define CYLINDER 1
#define RECTANGULARPRISM 2

// Object sınıfıyla bir vücut parçası

#define ROOT_OBJECT true
#define RELATIVE_OBJECT false

// Bir eklem'e dönme komutu verilirken
// eksenin belirtilmesi için aşağıdaki sabitler
// kullanılır.

#define X 0
#define Y 1
#define Z 2

// Açıları, koordinatları ve renkleri saklamak için

typedef struct coordinates
{
    double x, y, z;
} Coordinates;
typedef struct angles
{
    double x, y, z;
} Angles;
typedef struct rgba
{
    double red, green, blue, alpha;
} RGBA;

/////////////////////////////////////////////////////////////////// KAMERA & IŞIK

/*
Camera sınıfı kamera için pozisyon ve bakış bilgilerini
tutar, bu değerleri değiştirebilir veya güncelleyebilir.
Bir nesnenin etrafında dönmek(XZ düzleminde), pozisyonu
belli bir eksende ilerletmek, bir noktaya göndermek için
metodlar içerir.

GLHandler'ın display metodunda kullanılacak Camera::update
metodu OpenGL'e kameranın bilgilerini gönderir.
*/

class Camera
{
private:
    double positionX, positionY, positionZ;
    double lookX, lookY, lookZ;
    double upX, upY, upZ;

protected:
    double getRadiusXYZ(void)
    {
        return pow(pow(lookX - positionX, 2) +
                       pow(lookY - positionY, 2) +
                       pow(lookZ - positionZ, 2),
                   0.5);
    }
    double getRadiusXZ(void)
    {
        return pow(pow(lookX - positionX, 2) +
                       pow(lookZ - positionZ, 2),
                   0.5);
    }
    double getAngleXZ(void)
    {
        double res = atan2(positionZ - lookZ, positionX - lookX) * 180.0 / PI;
        if (res < 0)
            return res + 360;
        else
            return res;
    }

public:
    Camera(void)
    {
        positionX = positionY = positionZ = 0;
        lookX = lookY = lookZ = 0;
        upX = upZ = 0;
        upY = 1;
    }

    void update(void)
    {
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        gluLookAt(
            positionX, positionY, positionZ,
            lookX, lookY, lookZ,
            upZ, upY, upZ);
    }

    void setPosition(double x, double y, double z)
    {
        positionX = x;
        positionY = y;
        positionZ = z;
    }
    void setX(double x)
    {
        positionX = x;
    }
    void setY(double y)
    {
        positionY = y;
    }
    void setZ(double z)
    {
        positionZ = z;
    }
    void setOrigin(double x, double y, double z)
    {
        lookX = x;
        lookY = y;
        lookZ = z;
    }
    void translateOrigin(double x, double y, double z)
    {
        lookX += x;
        lookY += y;
        lookZ += z;
    }
    void translateX(double increase)
    {
        positionX = positionX + increase;
    }
    void translateY(double increase)
    {
        positionY = positionY + increase;
    }
    void translateZ(double increase)
    {
        positionZ = positionZ + increase;
    }
    void rotateXZ(double increase)
    {
        /*
		Bakılan konum çevresinde (Y ekseninin
		normu olduğu XZ düzleminde) kamerayı döndürür.
		*/
        double currentRadius = getRadiusXZ();
        double targetAngle_degree = getAngleXZ() + increase;
        double targetAngle_radian = targetAngle_degree * PI / 180.0;
        positionX = cos(targetAngle_radian) * currentRadius;
        positionZ = sin(targetAngle_radian) * currentRadius;
    }
};

/*
Işık sınıfı, ışık için pozisyon bilgilerini tutar ve
GLHandler::display metodunun kullandığı Light::update
metodunu içerir. Bu metod ışığın son konumunu OpenGL'e
bildirir.
*/

class Light
{
private:
    Coordinates light0;

public:
    Light(void)
    {
        light0.x = 2;
        light0.y = 2;
        light0.z = 2;
    }
    void init(void)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);

        glColorMaterial(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE);

        GLfloat light0_amb[] = {0.2, 0.2, 0.2, 1.0};
        GLfloat light0_dif[] = {0.8, 0.8, 0.8, 1.0};
        glLightfv(GL_LIGHT0, GL_AMBIENT, light0_amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_dif);
    }
    void update(void)
    {
        GLfloat light0_pos[] = {light0.x, light0.y, light0.z, 0.0};
        glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    }
};

/////////////////////////////////////////////////////////////////// VÜCUT MODELİ

class Object
{

private:
    int shape;   //  RECTANGULARPRISM  CYLINDER    SPHERE
    double dim1; //  width             radius	  radius
    double dim2; //  height            height      -
    double dim3; //  depth             -           -

    void draw(void)
    {
        // Bu metod, update metodunun içinden çağrılır.
        // update metodu cismin merkezine gelmesi gereken yeri
        // orijine denk getirmiştir. Cismin tipine göre çizim
        // gerçekleştirilir.

        if (shape == RECTANGULARPRISM)
        {
            glScaled(dim1, dim2, dim3);
            glutSolidCube(1.0);
        }
        else if (shape == CYLINDER)
        {
            GLUquadricObj *quadratic;
            quadratic = gluNewQuadric();

            // Silindir OpengGL tarafından varsayılan olarak
            // orijinden z pozitife uzanacak şekilde çizildiği
            // için istenilen doğrultuya denk getirmek amacıyla
            // iç döndürme yapılıyor. (Eklem döndürmesinden farklıdır)
            glRotated(rotate.x, 1, 0, 0);
            glRotated(rotate.y, 0, 1, 0);
            glRotated(rotate.z, 0, 0, 1);
            glTranslated(0, 0, -dim2 / 2); // Orta noktadan dönmesi için

            gluCylinder(quadratic, dim1, dim1, dim2, 64, 64);
        }
        else if (shape == SPHERE)
        {
            glutSolidSphere(this->dim1, 128, 128);
        }
    }

    // Cismin rengi ve iç döndürmesi (eklem dönmesiyle alakasız-silindir için kullanılıyor)

    RGBA color;
    Angles rotate;

    // Cisim başka bir cisme bağlanmıyorsa TRUE

    bool rootObject;

    // Cismin bağlı olduğu eksenin cismin ortasına göre koordinatı

    Coordinates offsetOfJointToParent;

public:
    // Taşınan cisimler için o cisimlerin pointer'ları,
    // ve o cisimlerle eklemlerinin açı ve offset bilgileri

    std::vector<Object *> children;
    std::vector<Angles> jointAngles;
    std::vector<Coordinates> jointOffsets;

    // Constructor

    Object(int shape, bool rootObject = false)
    {
        // Cismin şeklini işaretlemek
        this->shape = shape;

        // Cismin bağlanma durumu
        this->rootObject = rootObject;
        if (rootObject)
        {
            // Cisim başka bir cisme bağlanmıyorsa bağlanma değerleri 0 işaretlenir.
            // (update metodu için gerekli)
            Coordinates offsetOfJointToParent;
            offsetOfJointToParent.x = offsetOfJointToParent.y = offsetOfJointToParent.z = 0;
        }
    }

    void set(
        double widthOrRadius, double height, double depth,
        double red, double green, double blue,
        double rotateX, double rotateY, double rotateZ)
    {
        // 1.2.3. argümanlar cismin ölçüleridir.
        // Şekil dikdörtgenler prizmasıysa
        // width-height-depth olarak kullanılır,
        // silindirse radius ve height olarak kullanılır,
        // küreyse radius olarak kullanılır. Gerek olmayan
        // değerler yerine 0 verilerek çağrılır.

        // 4.5.6 argümanlar renktir.

        // 7.8.9. argümanlar iç döndürmedir. (eklem
        // döndürmesindenbağımsız)

        this->dim1 = widthOrRadius;
        if (shape != SPHERE)
        {
            this->dim2 = height;
            if (shape != CYLINDER)
            {
                this->dim3 = depth;
            }
        }

        this->color.red = red;
        this->color.green = green;
        this->color.blue = blue;

        this->rotate.x = rotateX;
        this->rotate.y = rotateY;
        this->rotate.z = rotateZ;
    }

    void link(
        Object &child,
        double jointAngleX, double jointAngleY, double jointAngleZ,
        double parentOffsetX, double parentOffsetY, double parentOffsetZ,
        double childOffsetX, double childOffsetY, double childOffsetZ)
    {
        /*
		a.link(b, açı, a-ofset, b-ofset);
		a nesnesi ana organdır.
		b nesnesi a'ya bağlı organdır.
		(2.3.4. parametre) b'nin a'ya göre duruş açısı (yada eklem açısı denebilir)
		(5.6.7. parametre) a-ofset, a'nın merkezi orijin kabul edildiğinde eklem noktasının koordinatıdır
		(8.9.10. parametre) b-ofset, b'nin merkezi orijin kabul edildiğinde eklem noktasının koordinatıdır

		Object tipindeki A nesnesi(ana nesne) yine Object tipindeki
		B nesnesine(bağlanan nesne) bağlanabilir. Bu bağlanma eklem
		noktasını temsil eder. A'nın, A'nın hangi noktasından, B'nin
		hangi noktasına hangi açıyla bağlanacağı belirtilir. Bu
		bilgiler A ve B nesnesine kaydedilir. A'daki 3 vektör, A'ya
		bağlanan B gibi tüm nesnelerin bilgilerini tutar. B'deki
		offsetOfParentObject değişkeni eklem noktasının B'nin merkezine
		olan uzaklığını saklar ve bu metodun işlevi biter.
		*/

        this->children.push_back(&child);

        Angles jointAngle = {jointAngleX, jointAngleY, jointAngleZ};
        this->jointAngles.push_back(jointAngle);

        Coordinates parentOffset = {parentOffsetX, parentOffsetY, parentOffsetZ};
        this->jointOffsets.push_back(parentOffset);

        Coordinates childOffset = {childOffsetX, childOffsetY, childOffsetZ};
        child.offsetOfJointToParent = childOffset;

        return;
    }

    void update(void)
    {
        // Cismin renk bilgileri OpenGL'e iletilir.

        glColor3d(this->color.red, this->color.green, this->color.blue);

        // Bu metod gövde için çağrıldığında translate ile 0,0,0 kaydırması
        // yapılıp gövde çizilir. Gövdeye bağlı bir cisim için çağrıldığında
        // eklemin cismin merkezine göre koordinatı (offset) kadar kaydırma
        // yapılıp orijini cismin çizilmesi gereken noktaya kaydırır.

        glTranslated(
            offsetOfJointToParent.x,
            offsetOfJointToParent.y,
            offsetOfJointToParent.z);

        // Mevcut konumlar (matris) saklanarak draw metodu çağrılır.
        // (Silindir çiziminde rotate ve translate yapıldığı için push-pop
        // gerekli.)

        glPushMatrix();
        this->draw();
        glPopMatrix();

        // Cismin çizimi bu noktada bitmiştir. Mevcut konum çizilmiş cismin
        // ortasını gösteriyordur.

        // Bu metod cismin çizimi bittiğinde bağlı cisimler için tekrar bu
        // fonksiyonu çağırır. Ancak önce eklem noktasına ilerler.

        // Aşağıdaki for döngüsü cisme bağlı cisim sayısı kadar tekrarlanır.
        // Örneğin gövde için bağlı cisimler sol-omuz, sağ-omuz, sol-kalça,
        // sağ-kalça ve boyundur. Yani 5 döngü gerçekleşir. Tabi sol-omuz için
        // bu fonksiyon çağrıldığında ona bağlı olan 1 cisim vardır(sol-kol).
        // Sonra sol-dirsek, sol-önkol için çağrılır ve çağrılar sonlanıp
        // sağ-omuza(ikinci döngü) geçer.

        for (unsigned int i = 0, length = this->children.size(); i < length; i++)
        {
            // Bir cisme bağlı birden fazla eklem olabileceği için her
            // eklem için kaydırma yapmadan önce currentmatrix saklanıyor
            // ve bir sonraki ekleme geçilmeden önce pop ile saklı matrix'e
            // geri dönülüyor.

            glPushMatrix();

            // Bağlı cismin bağlanma noktasına kaydırma.
            Coordinates offsets = this->jointOffsets[i];
            glTranslated(offsets.x, offsets.y, offsets.z);

            // Cismin bağlanma açısı (eklem açısı)
            // 3 eksende dönme yapılıyor
            Angles angles = this->jointAngles[i];
            glRotated(angles.x, 1, 0, 0);
            glRotated(angles.y, 0, 1, 0);
            glRotated(angles.z, 0, 0, 1);

            // Bu noktada çizilen cisim ile çizilecek cismin
            // kesişme noktasına translate ve eklem açısına rotate
            // ile gelindi. Çizilecek cisim için aynı metod çağrılıyor.

            this->children[i]->update();

            // Daha fazla birbirine uç uca bağlı cisim kalmadığında
            // aynı cisme bağlı birden fazla cisim olabileceği için pop
            // yapılıyor ve saklı konuma dönülüyor.
            glPopMatrix();
        }

        return;
    }
};

// Human metodları herhangi bir vücut parçasının
// bilgilerini (eklem açısı vs.) güncellemek istediğinde
// Object sınıfının ilgili metoduna aşağıdaki sabitler ile
// komut gönderir.

#define BODY 0

#define HEAD 1
#define NECK 2
#define LEFT_ARM 3
#define LEFT_FOREARM 4
#define LEFT_FOOT 5
#define RIGHT_ARM 6
#define RIGHT_FOREARM 7
#define RIGHT_FOOT 8

#define LEFT_SHOULDER 9
#define LEFT_ELBOW 10
#define LEFT_HIP 11
#define RIGHT_SHOULDER 12
#define RIGHT_ELBOW 13
#define RIGHT_HIP 14

class Human
{
private:
    // Object tipinde tüm vücut parçaları ve
    // eklemler için nesneler oluşturuluyor.

    // Human::init içinde bu nesneler birbiriyle
    // parent-child ilişkisiyle bağlanacak.

    Object
        body,
        head,
        neck,
        leftEyeOutside,
        leftEyeInside,
        leftArm,
        leftForearm,
        leftFoot,
        leftShoulder,
        leftElbow,
        leftHip,
        rightEyeOutside,
        rightEyeInside,
        rightArm,
        rightForearm,
        rightFoot,
        rightShoulder,
        rightElbow,
        rightHip;

    // Tüm iskeletin durma noktası ve açısı

    Coordinates mainPosition;
    Angles mainAngle;

    // Animasyonlar için açık-kapalı durumunu gösteren bool'lar
    // Animasyonun döngüsünü tamamlama yüzdesi double'lar
    // Animasyonun toplam kaç kare süreceğini gösteren double'lar (animasyonun hızını belirliyor)

    bool walking;
    double walkingCompletionPercent;
    double walkingTotalAnimationIteration;

    bool waving;
    double wavingCompletionPercent;
    double wavingTotalAnimationIteration;

    bool roaming;
    double roamingCompletionPercent;
    double roamingTotalAnimationIteration;

public:
    // Human Constructor'ı member variable
    // olan Object nesnelerinin constructor'larını
    // çağırır ve bu esnada bu cisimlerin silindir-küre
    // olma durumunu da işaretler.

    Human(void)
        : body(CYLINDER, ROOT_OBJECT)

          ,
          head(SPHERE), leftEyeOutside(SPHERE), leftEyeInside(SPHERE), rightEyeOutside(SPHERE), rightEyeInside(SPHERE), neck(CYLINDER)

          ,
          leftArm(CYLINDER), leftForearm(CYLINDER), leftFoot(CYLINDER)

          ,
          rightArm(CYLINDER), rightForearm(CYLINDER), rightFoot(CYLINDER)

          ,
          leftShoulder(SPHERE), leftElbow(SPHERE), leftHip(SPHERE)

          ,
          rightShoulder(SPHERE), rightElbow(SPHERE), rightHip(SPHERE)
    {
        walkingCompletionPercent = 0;
        wavingCompletionPercent = 0;
        walking = false;
        waving = false;
        roaming = false;

        mainPosition = {0, -0.07, 0};
        mainAngle = {0, 0, 0};
        return;
    }
    void init(void)
    {
        // Bu metod OpenGL ile ilk defa iletişim kurulduğunda
        // çağrılır ve her Human nesnesi için bir kere çalışır.

        // Görevi her vücut parçası için ölçü, renk, iç döndürme
        // tanımlamalarını yapar ve parçaları parent-child ilişkisine
        // göre linkler.

        // PARÇALARIN OLUŞTURULMASI, ÖLÇÜLENDİRİLMESİ

        rightEyeOutside.set(
            0.1, 0, 0, // yarıçap / boş / boş
            1, 1, 1,   // color
            0, 0, 0    // rotation
        );
        rightEyeInside.set(
            0.04, 0, 0,    // yarıçap / yükseklik / boş
            0.5, 0.3, 0.1, // color
            0, 0, 0        // rotation
        );

        leftEyeOutside.set(
            0.1, 0, 0, // yarıçap / boş / boş
            1, 1, 1,   // color
            0, 0, 0    // rotation
        );
        leftEyeInside.set(
            0.04, 0, 0,    // yarıçap / yükseklik / boş
            0.5, 0.3, 0.1, // color
            0, 0, 0        // rotation
        );

        head.set(
            0.5, 0, 0, // yarıçap / boş / boş
            1, 0.6, 0, // color
            0, 0, 0    // rotation
        );

        neck.set(            // yarıçap / yükseklik / boş
            0.1, 0.2, 0,     // color
            0.13, 0.26, 1.0, // rotation
            90, 0, 0);

        body.set(
            0.5, 1.3, 0, // yarıçap / yükseklik / boş
            1, 0.6, 0,   // color
            90, 0, 0     // rotation
        );

        rightShoulder.set(
            0.1001, 0, 0, // yarıçap / boş / boş
            1, 0, 0,      // color
            0, 0, 0       // rotation
        );
        rightArm.set(
            0.1, 0.7, 0,      // yarıçap / yükseklik / boş
            0.12, 0.38, 0.25, // color
            0, 90, 0          // rotation
        );
        rightElbow.set(
            0.1001, 0, 0, // yarıçap / boş / boş
            1, 0, 0,      // color
            0, 0, 0       // rotation
        );
        rightForearm.set(
            0.1, 0.7, 0, // yarıçap / yükseklik / boş
            1, 1, 0,     // color
            0, 90, 0     // rotation
        );

        leftShoulder.set(
            0.1001, 0, 0, // yarıçap / boş / boş
            1, 0, 0,      // color
            0, 0, 0       // rotation
        );
        leftArm.set(
            0.1, 0.7, 0,      // yarıçap / yükseklik / boş
            0.12, 0.38, 0.25, // color
            0, 90, 0          // rotation
        );
        leftElbow.set(
            0.1001, 0, 0, // yarıçap / boş / boş
            1, 0, 0,      // color
            0, 0, 0       // rotation
        );
        leftForearm.set(
            0.1, 0.7, 0, // yarıçap / yükseklik / boş
            1, 1, 0,     // color
            0, 90, 0     // rotation
        );

        rightHip.set(
            0.1001, 0, 0, // yarıçap / boş / boş
            1, 0, 0,      // color
            0, 0, 0       // rotation
        );
        rightFoot.set(
            0.1, 1.0, 0,      // yarıçap / yükseklik / boş
            0.12, 0.38, 0.25, // color
            90, 0, 90         // rotation
        );

        leftHip.set(
            0.1001, 0, 0, // yarıçap / boş / boş
            1, 0, 0,      // color
            0, 0, 0       // rotation
        );
        leftFoot.set(
            0.1, 1.0, 0,      // yarıçap / yükseklik / boş
            0.12, 0.38, 0.25, // color
            90, 0, 90         // rotation
        );

        // PARÇALARIN BİRLEŞTİRİLMESİ (İSKELET)

        body.link(         // parent
            rightShoulder, // child
            0, 0, 0,       // eklem açısı
            -0.5, 0.3, 0,  // parent offset
            0, 0, 0        // child offset
        );
        rightShoulder.link( // parent
            rightArm,       // child
            0, 0, 6,        // eklem açısı
            0, 0, 0,        // parent offset
            -0.35, 0, 0     // child offset
        );
        rightArm.link(   // parent
            rightElbow,  // child
            0, 0, 0,     // eklem açısı
            -0.35, 0, 0, // parent offset
            0, 0, 0      // child offset
        );
        rightElbow.link(  // parent
            rightForearm, // child
            0, 0, -90,    // eklem açısı
            0, 0, 0,      // parent offset
            -0.35, 0, 0   // child offset
        );

        body.link(        // parent
            leftShoulder, // child
            0, 0, 0,      // eklem açısı
            0.5, 0.3, 0,  // parent offset
            0, 0, 0       // child offset
        );
        leftShoulder.link( // parent
            leftArm,       // child
            0, 0, -6,      // eklem açısı
            0, 0, 0,       // parent offset
            0.35, 0, 0     // child offset
        );
        leftArm.link(   // parent
            leftElbow,  // child
            0, 0, 0,    // eklem açısı
            0.35, 0, 0, // parent offset
            0, 0, 0     // child offset
        );
        leftElbow.link(  // parent
            leftForearm, // child
            0, 0, 90,    // eklem açısı
            0, 0, 0,     // parent offset
            0.35, 0, 0   // child offset
        );

        body.link(          // parent
            leftHip,        // child
            0, 0, -10,      // eklem açısı
            -0.2, -0.65, 0, // parent offset
            0, 0, 0         // child offset
        );
        leftHip.link(  // parent
            leftFoot,  // child
            0, 0, 0,   // eklem açısı
            0, 0, 0,   // parent offset
            0, -0.5, 0 // child offset
        );

        body.link(         // parent
            rightHip,      // child
            0, 0, 10,      // eklem açısı
            0.2, -0.65, 0, // parent offset
            0, 0, 0        // child offset
        );
        rightHip.link( // parent
            rightFoot, // child
            0, 0, 0,   // eklem açısı
            0, 0, 0,   // parent offset
            0, -0.5, 0 // child offset
        );

        body.link(      // parent
            neck,       // child
            0, 0, 0,    // eklem açısı
            0, 0.65, 0, // parent offset
            0, 0.1, 0   // child offset
        );
        neck.link(     // parent
            head,      // child
            0, 0, 0,   // eklem açısı
            0, 0.1, 0, // parent offset
            0, 0.5, 0  // child offset
        );

        head.link(           // parent
            rightEyeOutside, // child
            0, 0, 0,         // eklem açısı
            -0.2, 0.1, 0.4,  // parent offset
            0, 0, 0          // child offset
        );
        rightEyeOutside.link( // parent
            rightEyeInside,   // child
            0, 0, 0,          // eklem açısı
            0, 0, 0,          // parent offset
            -0.01, 0.01, 0.1  // child offset
        );

        head.link(          // parent
            leftEyeOutside, // child
            0, 0, 0,        // eklem açısı
            0.2, 0.1, 0.4,  // parent offset
            0, 0, 0         // child offset
        );
        leftEyeOutside.link( // parent
            leftEyeInside,   // child
            0, 0, 0,         // eklem açısı
            0, 0, 0,         // parent offset
            0.01, 0.01, 0.1  // child offset
        );
    }
    void update(void)
    {
        // Sahneye çizilecek diğer nesneler için vücudun çizimi sırasında
        // kullanılacak rotate-scale-translate'den sonra koordinatların
        // değişme için push-pop kullanılıyor.

        glPushMatrix();

        // Dolaşma animasyonu açıksa iskeletin konumunu animasyon belirler
        // (Eğer kapalıysa değişiklik yapmadan return ediyor.)
        roamingAnimation();

        // İskeletin E-S-D-F ile dolaşması için
        glTranslated(mainPosition.x, mainPosition.y, mainPosition.z);
        glRotated(mainAngle.x, 1, 0, 0);
        glRotated(mainAngle.y, 0, 1, 0);
        glRotated(mainAngle.z, 0, 0, 1);

        // wave-walk animasyonları cisim çizilmeden önce çalışıp
        // eklem eğimlerini düzenliyor.
        waveAnimation();
        walkAnimation();

        glPushMatrix();

        // Zemine batmaması için model yükseltiliyor.
        glTranslated(0.0, 1.7, 0.0);

        // Gövde nesnesi için çizim fonksiyonu çağrılıyor. Bu metod, kendisine
        // bağlı cisimler için içinden çağrılacak ve tüm vücut çizilmiş olacak.
        body.update();

        glPopMatrix();

        glPopMatrix();
    }

    void setMainCoordinates(double x, double y, double z)
    {
        // Modelin koordinatlarını seçer
        mainPosition.x = x;
        mainPosition.y = y;
        mainPosition.z = z;
    }
    void raiseMainCoordinates(double x, double y, double z)
    {
        // Modelin koordinatlarını düzenler
        mainPosition.x += x;
        mainPosition.y += y;
        mainPosition.z += z;
    }

    void raiseAngle(int partNumber, int direction, double angle)
    {
        // Bu metod spesifik bir vücut parçası için ilgili ekleminin
        // dönüş değerlerini artırıp azaltıyor.
        switch (partNumber)
        {
        case LEFT_ARM:
            if (direction == X)
                leftShoulder.jointAngles[0].x += angle;
            else if (direction == Y)
                leftShoulder.jointAngles[0].y += angle;
            else if (direction == Z)
                leftShoulder.jointAngles[0].z += angle;
            break;
        case LEFT_FOREARM:
            if (direction == X)
                leftElbow.jointAngles[0].x += angle;
            else if (direction == Y)
                leftElbow.jointAngles[0].y += angle;
            else if (direction == Z)
                leftElbow.jointAngles[0].z += angle;
            break;
        case RIGHT_ARM:
            if (direction == X)
                rightShoulder.jointAngles[0].x += angle;
            else if (direction == Y)
                rightShoulder.jointAngles[0].y += angle;
            else if (direction == Z)
                rightShoulder.jointAngles[0].z += angle;
            break;
        case RIGHT_FOREARM:
            if (direction == X)
                rightElbow.jointAngles[0].x += angle;
            else if (direction == Y)
                rightElbow.jointAngles[0].y += angle;
            else if (direction == Z)
                rightElbow.jointAngles[0].z += angle;
            break;
        case RIGHT_FOOT:
            if (direction == X)
                rightHip.jointAngles[0].x += angle;
            else if (direction == Y)
                rightHip.jointAngles[0].y += angle;
            else if (direction == Z)
                rightHip.jointAngles[0].z += angle;
            break;
        case LEFT_FOOT:
            if (direction == X)
                leftHip.jointAngles[0].x += angle;
            else if (direction == Y)
                leftHip.jointAngles[0].y += angle;
            else if (direction == Z)
                leftHip.jointAngles[0].z += angle;
            break;
        }
    }
    void setAngle(int partNumber, int direction, double angle)
    {
        // Bu metod spesifik bir vücut parçası için ilgili ekleminin
        // dönüş değerlerini değiştiriyor.
        switch (partNumber)
        {
        case LEFT_ARM:
            if (direction == X)
                leftShoulder.jointAngles[0].x = angle;
            else if (direction == Y)
                leftShoulder.jointAngles[0].y = angle;
            else if (direction == Z)
                leftShoulder.jointAngles[0].z = angle;
            break;
        case LEFT_FOREARM:
            if (direction == X)
                leftElbow.jointAngles[0].x = angle;
            else if (direction == Y)
                leftElbow.jointAngles[0].y = angle;
            else if (direction == Z)
                leftElbow.jointAngles[0].z = angle;
            break;
        case RIGHT_ARM:
            if (direction == X)
                rightShoulder.jointAngles[0].x = angle;
            else if (direction == Y)
                rightShoulder.jointAngles[0].y = angle;
            else if (direction == Z)
                rightShoulder.jointAngles[0].z = angle;
            break;
        case RIGHT_FOREARM:
            if (direction == X)
                rightElbow.jointAngles[0].x = angle;
            else if (direction == Y)
                rightElbow.jointAngles[0].y = angle;
            else if (direction == Z)
                rightElbow.jointAngles[0].z = angle;
            break;
        case RIGHT_FOOT:
            if (direction == X)
                rightHip.jointAngles[0].x += angle;
            else if (direction == Y)
                rightHip.jointAngles[0].y += angle;
            else if (direction == Z)
                rightHip.jointAngles[0].z += angle;
            break;
        case LEFT_FOOT:
            if (direction == X)
                leftHip.jointAngles[0].x += angle;
            else if (direction == Y)
                leftHip.jointAngles[0].y += angle;
            else if (direction == Z)
                leftHip.jointAngles[0].z += angle;
            break;
        }
    }

    void startWalking(unsigned int a = 128)
    {
        walkingTotalAnimationIteration = a;
        walking = true;
    }
    void stopWalking(void)
    {
        walking = false;
    }
    void toggleWalking(void)
    {
        // ilk tıkta yürüme animasyonu başlatıyor,
        // ikinci tıkta dolaşma animasyonu da başlatıyor,
        // üçüncü tıkta ikisi de kapatılıyor
        if (walking)
        {
            if (roaming)
            {
                stopWalking();
                toggleRoaming();
            }
            else
                startRoaming();
        }
        else
            startWalking();
    }
    void walkAnimation(void)
    {
        if (!walking)
            return;

        // Animasyonun her karesi için animasyonun tamamlanma yüzdesi
        // cosinus fonksiyonundan geçirilerek animasyon lineer değil sinusoidal
        // hale getiriliyor (ease-in-out)

        // frameAngleX -2 ile 2 arasında değerler üretiyor. (tamamlanma yüzdesine göre)
        // framePositionY -0.07 ile -0.08 arasında değerler üretiyor.

        double frameAngleX = 2 * std::cos(walkingCompletionPercent * 360 * PI / 180);
        double framePositionY = 0.05 * std::cos(walkingCompletionPercent * 2 * 360 * PI / 180) - 0.075;
        // bu fonksiyonun periyodu ilk fonksiyonun periyodunun yarısı kadar

        // Sol ve sağ bacak frameAngleX'in zıt işaretlileriyle eğiliyor.

        setAngle(RIGHT_FOOT, X, frameAngleX);
        setAngle(LEFT_FOOT, X, -frameAngleX);

        // Modelin adım atma sırasında yükselip alçalması için;

        mainPosition.y = framePositionY;

        // Animasyon çağrılırken belirtilen, animasyonun bir döngüsünün gerçekleşeceği
        // toplam kare sayısının tersi alınarak tamamlanma yüzdesi arttırılıyor.

        walkingCompletionPercent += (1.0 / walkingTotalAnimationIteration);
        if (walkingCompletionPercent >= 1.0)
            walkingCompletionPercent -= 1.0;
    }

    void startRoaming(unsigned int a = 512)
    {
        roamingTotalAnimationIteration = a;
        roaming = true;
    }
    void stopRoaming(void)
    {
        roaming = false;
        mainPosition.z = mainPosition.x = 0;
        mainAngle.y = 0;
    }
    void toggleRoaming(void)
    {
        if (roaming)
            stopRoaming();
        else
            startRoaming();
    }
    void roamingAnimation(void)
    {
        if (!roaming)
            return;

        // dolaşma animasyonu modelin zeminde tur atmasını sağlıyor
        // x ve z ekseninin alacağı değerler aynı fonksiyonun t(zaman)
        // ekseninde çeyrek periyot kaydırılmasıyla bulunuyor.

        double framePositionZ = 3 * std::sin(roamingCompletionPercent * 360 * PI / 180);
        mainPosition.z = framePositionZ;

        double framePositionX = 3 * std::sin((roamingCompletionPercent + 0.25) * 360 * PI / 180);
        mainPosition.x = framePositionX;

        // Modelin önünün sürekli dönmesi gerekiyor. (Lineer zamanlamalı bir animasyon olduğu için sin/cos yok)
        mainAngle.y = -roamingCompletionPercent * 360.0;

        roamingCompletionPercent += (1.0 / roamingTotalAnimationIteration);
        if (roamingCompletionPercent >= 1.0)
            roamingCompletionPercent -= 1.0;
    }

    void startWaving(unsigned int a = 48)
    {
        wavingTotalAnimationIteration = a;
        waving = true;
    }
    void stopWaving(void)
    {
        waving = false;
    }
    void toggleWaving(void)
    {
        if (waving)
            stopWaving();
        else
            startWaving();
    }
    void waveAnimation(void)
    {
        if (!waving)
            return;

        // El sallama animasyonu ease-in-out zamanlamalı bir animasyon olması
        // gerektiği için sin/cos fonksiyonu kullanılıyor. (cos'un tercih sebebi 1.00'dan başlaması)

        double frameAngleZ = -20 * std::cos(wavingCompletionPercent * 360 * PI / 180);

        // Ön kol ve arka kol frameAngleZ'nin farklı katsayılarla
        // ölçeklendirilmesiyle elde edilen değerlerle eğiliyor.
        setAngle(LEFT_ARM, Z, -frameAngleZ * 0.2 - 10);
        setAngle(LEFT_FOREARM, Z, -frameAngleZ + 70);

        // BONUS: Toggle the code below instead of above
        // setAngle(LEFT_ARM, Z, -frameAngleZ * 1.5);
        // setAngle(LEFT_FOREARM, Z, frameAngleZ * 2);
        // setAngle(RIGHT_ARM, Z, -frameAngleZ * 1.5);
        // setAngle(RIGHT_FOREARM, Z, frameAngleZ * 2);

        wavingCompletionPercent += (1.0 / wavingTotalAnimationIteration);
        if (wavingCompletionPercent >= 1.0)
            wavingCompletionPercent -= 1.0;
    }
};

/////////////////////////////////////////////////////////////////// ANA SINIF

class GLHandler
{
private:
    // Işık, kamera ve iskelet işlemlerini üstlenen değişkenler
    Light light;
    Camera camera;
    Human model1;

public:
    void init(void)
    {
        // Kamera perspektif ayarı

        glMatrixMode(GL_PROJECTION);                   // Perspektif için
        glLoadIdentity();                              // Birim matris
        gluPerspective(20, 1600.0 / 900.0, 0.1, 1000); // açı, oran, yakın, uzak
        glMatrixMode(GL_MODELVIEW);                    // Sahne çizimi için

        // Kameranın bakış açısında engelin
        // arkasında kalan cisimlerin çizilmemesi için

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Işıktan sorumlu sınıfın ilk çalıştırma ayarlarının uygulanması

        light.init();

        // Kamera için konum ve bakış noktası ayarı

        camera.setOrigin(0, 1.5, 0);
        camera.setPosition(0, 3, 20);

        // Modeli canlandıran sınıfın ilk çalıştırma ayarlarının uygulanması
        //          (vücut parçalarının uzunlukları, eklemlerin başlangıç
        //           açıları ve ilgili parçaların birbirine kenetlenmesini
        //           sağlar, ayrıntılı açıklama Human sınıfının içindedir)

        model1.init();
    }
    void display(void)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // Kameranın güncel konumunu OpenGL'e bildirir
        camera.update();

        // Işığın güncel konumunu OpenGL'e bildirir
        light.update();

        // Sahnedeki sabit modelleri çizer (yürümenin hissedilmesi için varlar)
        drawStaticModels();

        // İskeleti güncel haliyle çizdirir. (animasyonları bu sınıf üstleniyor)
        model1.update();

        glutSwapBuffers();
    }
    void drawStaticModels(void)
    {
        // mor kutu
        glPushMatrix();
        glColor3d(1.0, 0.6, 1.0);
        glTranslated(-1.0, 0.15, -1.0);
        glRotated(60, 0, 1, 0);
        glutSolidCube(0.3);
        glPopMatrix();
        // mavi kutu
        glPushMatrix();
        glColor3d(0.6, 1.0, 1.0);
        glTranslated(1.0, 0.35, 1.0);
        glRotated(30, 0, 1, 0);
        glutSolidCube(0.7);
        glPopMatrix();
        // demlik
        glPushMatrix();
        glColor3d(0.5, 0.2, 0);
        glTranslated(1.0, 0.95, 1.0);
        glutSolidTeapot(0.3);
        glPopMatrix();
        // zemin
        glPushMatrix();
        glColor3d(1, 1, 1);
        glScaled(10.0, 0.05, 10.0);
        glutSolidCube(1.0);
        glPopMatrix();
    }
    void keyboard(unsigned char key, int x, int y)
    {
        // Ekstra olan klavye kısayolları
        // Gerekli olanlar bir alttaki metotta

        switch (key)
        {

        case 'q':
            model1.raiseAngle(LEFT_FOREARM, Z, -1);
            break;
        case 'Q':
            model1.raiseAngle(LEFT_FOREARM, Z, 1);
            break;
        case 'w':
            model1.raiseAngle(LEFT_ARM, Z, -1);
            break;
        case 'W':
            model1.raiseAngle(LEFT_ARM, Z, 1);
            break;

        case 't':
            model1.raiseAngle(RIGHT_FOREARM, Z, 1);
            break;
        case 'T':
            model1.raiseAngle(RIGHT_FOREARM, Z, -1);
            break;
        case 'r':
            model1.raiseAngle(RIGHT_ARM, Z, 1);
            break;
        case 'R':
            model1.raiseAngle(RIGHT_ARM, Z, -1);
            break;

        case 'e':
        case 'E':
            model1.raiseMainCoordinates(0, 0, 0.1);
            break;
        case 'd':
        case 'D':
            model1.raiseMainCoordinates(0, 0, -0.1);
            break;
        case 's':
        case 'S':
            model1.raiseMainCoordinates(0.1, 0, 0);
            break;
        case 'f':
        case 'F':
            model1.raiseMainCoordinates(-0.1, 0, 0);
            break;

        case 'a':
            model1.raiseAngle(RIGHT_FOOT, X, -1);
            break;
        case 'A':
            model1.raiseAngle(RIGHT_FOOT, X, 1);
            break;
        }
    }
    void specialKeyboard(int key, int x, int y)
    {
        switch (key)
        {
        case GLUT_KEY_LEFT:
            camera.rotateXZ(2); // yatay kamera hareketi
            break;
        case GLUT_KEY_RIGHT:
            camera.rotateXZ(-2); // yatay kamera hareketi
            break;
        case GLUT_KEY_UP:
            camera.translateY(0.5); // dikey kamera hareketi
            break;
        case GLUT_KEY_DOWN:
            camera.translateY(-0.5); // dikey kamera hareketi
            break;
        }
    }
    void mouse(int button, int state, int x, int y)
    {
        if (state == GLUT_DOWN)
        { // farenin basılma anı
            switch (button)
            {
            case GLUT_LEFT_BUTTON:
                model1.toggleWaving(); // wave
                break;
            case GLUT_RIGHT_BUTTON:
                model1.toggleWalking(); // walk
                break;
            }
        }
    }
    void idle(void)
    {
        // input cihazlarından veriler alındığında sadece
        // nesnelerin bilgileri güncellenir. Yeniden çizme
        // işlemi frekans sabitlemesi açısından sadece idle
        // tarafından sağlanır.
        glutPostRedisplay();
    }
};

/////////////////////////////////////////////////////////////////// MAİN

// OpenGL callbacklerini ve bu callback'lerin ihtiyaç duyacağı
// tüm değişkenleri bu nesne barındıracak.
GLHandler gl;

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1600, 900);
    glutCreateWindow("github.com/ufukty - 2016");

    // Perspektif ayarı, depth ayarı, Camera::init çağrısı,
    // Light::init çağrısı ve Human::init çağrısı yapılıyor.
    gl.init();

    // GLHandler içindeki ilgili callback fonksiyonları çağıran
    // isimsiz(lambda) fonksiyonların OpenGL'e bildirilmesi. (C++11)
    glutDisplayFunc([](void) -> void { gl.display(); });
    glutKeyboardFunc([](unsigned char key, int x, int y) -> void { gl.keyboard(key, x, y); });
    glutSpecialFunc([](int key, int x, int y) -> void { gl.specialKeyboard(key, x, y); });
    glutMouseFunc([](int button, int state, int x, int y) -> void { gl.mouse(button, state, x, y); });
    glutIdleFunc([](void) -> void { gl.idle(); });

    glutMainLoop();
    return 0;
}