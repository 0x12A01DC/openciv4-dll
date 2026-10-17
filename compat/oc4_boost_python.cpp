#include "oc4_boost_python.h"

#include <pybind11/eval.h>

namespace py = pybind11;

namespace oc4::pycompat
{
	namespace
	{
		constexpr const char* kModuleName = "CvPythonExtensions";
		constexpr const char* kEnumRegistry = "__oc4_enum_types__";

		// Base for all exposed enums, mirroring Boost.Python's enum objects:
		// ints that print as their names and pickle as plain ints.
		constexpr const char* kEnumBaseSource = R"py(
class Oc4Enum(int):
    __slots__ = ()
    def __repr__(self):
        n = type(self)._oc4names.get(int(self))
        if n is None:
            return int.__repr__(self)
        return '%s.%s.%s' % (type(self).__module__, type(self).__name__, n)
    def __str__(self):
        n = type(self)._oc4names.get(int(self))
        if n is None:
            return int.__repr__(self)
        return n
    def __reduce__(self):
        return (int, (int(self),))
)py";

		py::object enumBase()
		{
			static PyObject* base = nullptr;
			if (!base)
			{
				py::dict ns;
				ns["__builtins__"] = py::module_::import("__builtin__");
				py::exec(kEnumBaseSource, ns);
				py::object cls = ns["Oc4Enum"];
				cls.attr("__module__") = kModuleName;
				base = cls.release().ptr();
			}
			return py::reinterpret_borrow<py::object>(base);
		}

		py::dict registry()
		{
			py::module_ m = currentModule();
			if (!py::hasattr(m, kEnumRegistry))
				m.attr(kEnumRegistry) = py::dict();
			return m.attr(kEnumRegistry);
		}
	}

	py::module_ currentModule()
	{
		return py::reinterpret_borrow<py::module_>(PyImport_AddModule(kModuleName));
	}

	py::object makeEnumType(const char* name)
	{
		py::dict attrs;
		attrs["__slots__"] = py::tuple();
		attrs["__module__"] = kModuleName;
		attrs["names"] = py::dict();
		attrs["values"] = py::dict();
		attrs["_oc4names"] = py::dict();
		py::object typeType = py::reinterpret_borrow<py::object>(reinterpret_cast<PyObject*>(&PyType_Type));
		return typeType(py::str(name), py::make_tuple(enumBase()), attrs);
	}

	void registerEnumType(const char* typeIdName, py::handle type)
	{
		registry()[py::str(typeIdName)] = type;
	}

	PyObject* findEnumType(const char* typeIdName)
	{
		PyObject* m = PyImport_AddModule(kModuleName);
		if (!m)
			return nullptr;
		PyObject* reg = PyObject_GetAttrString(m, kEnumRegistry);
		if (!reg)
		{
			PyErr_Clear();
			return nullptr;
		}
		PyObject* type = PyDict_GetItemString(reg, typeIdName); // borrowed; kept alive by the registry
		Py_DECREF(reg);
		return type;
	}

	PyObject* enumValue(PyObject* type, long value)
	{
		// Reuse the registered instance for known values, like Boost.Python.
		if (PyObject* values = PyObject_GetAttrString(type, "values"))
		{
			PyObject* key = PyInt_FromLong(value);
			PyObject* inst = PyDict_GetItem(values, key);
			Py_DECREF(key);
			Py_DECREF(values);
			if (inst)
			{
				Py_INCREF(inst);
				return inst;
			}
		}
		else
			PyErr_Clear();
		return PyObject_CallFunction(type, const_cast<char*>("l"), value);
	}
}
