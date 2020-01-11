.. Embedded Infrastructure Library documentation master file, created by
   sphinx-quickstart on Wed Nov 27 13:20:54 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to Embedded Infrastructure Library's documentation!
===========================================================

.. toctree::
    :maxdepth: 2
    :caption: Contents:

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

.. graphviz::

    digraph {
        "From" -> "To";
    }

    
Network Connections
===================

The data sent over TCP and similar streaming connections are handled via Connection and ConnectionObserver objects. A Connection object represents the network
towards which data is sent or received, a ConnectionObserver represents the application side handling sent or received data. The C++ wrapper for Lightweight IP
therefore supports a class ConnectionLwIP; the HttpClientImpl class derives from ConnectionObserver, since HttpClientImpl interprets received data, parses HTTP
headers, and forwards body datay.

The lifetime of a Connection/ConnectionObserver pair is under control of the network stack, since the network stacks knows when a connection is alive and when
it is terminated. The Connection object therefore derives from SharedOwnedObserver, which holds a SharedPtr to its SharedOwningSubject, which is the
ConnectionObserver. When a connection is closed, the Connection and ConnectionObserver objects 

.. uml::

    @startuml

    class SharedOwnedObserver {
        +bool IsAttached() const
        +SharedOwningSubject& Subject() const
        +void Detach()
        +{abstract}void Attached() {}
        +{abstract}void Detaching() {}
        -SharedOwningSubject* subject
    }

    class SharedOwningSubject {
        -SharedPtr<SharedOwnedObserver> observer
        +void Attach(const SharedPtr<SharedOwnedObserver>& observer)
        +void Detach()
        +bool IsAttached() const
        +SharedOwnedObserver& Observer() const
        +SharedPtr<SharedOwnedObserver> ObserverPtr() const
    }

    class ConnectionObserver{
        +{abstract}void SendStreamAvailable(SharedPtr<StreamWriter>&& streamWriter)
        +{abstract}void DataReceived()
        +{abstract}void Close() {...}
        +{abstract}void Abort() {...}
    }

    class Connection {
        +{abstract}void RequestSendStream(std::size_t sendSize)
        +{abstract}std::size_t MaxSendStreamSize() const
        +{abstract}SharedPtr<StreamReaderWithRewinding> ReceiveStream()
        +{abstract}void AckReceived()
        +{abstract}void CloseAndDestroy()
        +{abstract}void AbortAndDestroy()
    }

    SharedOwnedObserver <|-- ConnectionObserver 
    SharedOwningSubject <|-- Connection
    SharedOwnedObserver -left- SharedOwningSubject : < owns

    @enduml


