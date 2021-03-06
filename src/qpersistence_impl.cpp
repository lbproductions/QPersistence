#include "qpersistence.h"

#include "private.h"
#include "conversion.h"
#include "metaproperty.h"
#include "relationresolver.h"

#include <QSharedPointer>
#include <QWeakPointer>

namespace Qp {

template<class T>
void registerClass()
{
    static QObject guard;

    qRegisterMetaType<QSharedPointer<T> >();
    qRegisterMetaType<QList<QSharedPointer<T> > >();

    new QpDao<T>(&guard);

    // Create converter
    Private::ObjectConverter<T> *converter = new Private::ObjectConverter<T>(&guard);

    // Register converter for type
    Private::registerConverter<QList<QSharedPointer<T> > >(converter);

    // Register converter for list type
    Private::registerConverter<QSharedPointer<T> >(converter);
}

template <typename T>
QList<T> reversed( const QList<T> & in ) {
    QList<T> result;
    result.reserve( in.size() );
    std::reverse_copy( in.begin(), in.end(), std::back_inserter( result ) );
    return result;
}

template<class K, class V>
void registerMappableTypes()
{
    qRegisterMetaType<K>();
    qRegisterMetaType<V>();
    qRegisterMetaType<QMap<K,V> >();

    // Create converter
    static QObject guard;
    Private::registerConverter<QMap<K,V> >(new Private::MapConverter<K,V>(&guard));

    if(!Private::canConvertFromSqlStoredVariant<K>())
        Private::registerConverter<K>(new Private::SqlConverter<K>(&guard));
    if(!Private::canConvertFromSqlStoredVariant<V>())
        Private::registerConverter<V>(new Private::SqlConverter<V>(&guard));
}

template<class T>
void registerSetType()
{
    qRegisterMetaType<T>();
    qRegisterMetaType<QSet<T> >();

    // Create converter
    static QObject guard;
    Private::registerConverter<QSet<T> >(new Private::SetConverter<T>(&guard));

    if(!Private::canConvertFromSqlStoredVariant<T>())
        Private::registerConverter<T>(new Private::SqlConverter<T>(&guard));
}

template<class T> QSharedPointer<T> read(int id)
{
    return Qp::dataAccessObject<T>()->read(id);
}

template<class T>
QList<QSharedPointer<T> > readAll()
{
    return Private::castList<T>(dataAccessObject<T>()->readAllObjects());
}

template<class T>
int count()
{
    return dataAccessObject<T>()->count();
}

template<class T>
QSharedPointer<T> create()
{
    return qSharedPointerCast<T>(dataAccessObject<T>()->createObject());
}

template<class T>
QpDao<T> *dataAccessObject()
{
    return static_cast<QpDao<T> *>(QpDaoBase::forClass(T::staticMetaObject));
}

template<class T>
bool update(QSharedPointer<T> object)
{
    return QpDaoBase::forClass(*object->metaObject())->updateObject(object);
}

template<class T>
bool remove(QSharedPointer<T> object)
{
    return QpDaoBase::forClass(*object->metaObject())->removeObject(object);
}

template<class T>
QSharedPointer<T> sharedFrom(const T *object)
{
    QVariant variant = object->property(Qp::Private::QPERSISTENCE_SHARED_POINTER_PROPERTY.toLatin1());
    QWeakPointer<QObject> weak = variant.value<QWeakPointer<QObject> >();
    QSharedPointer<QObject> strong = weak.toStrongRef();
    return qSharedPointerCast<T>(strong);
}

template<class T>
int primaryKey(QSharedPointer<T> object)
{
    return Qp::Private::primaryKey(object.data());
}

} // namespace Qp
